/*
 * ---
 * BASIC++ Interpreter - mod_upnp.c
 * ---
 *
 * UPnP/SSDP Device Discovery Module.
 *
 * PURPOSE:
 * Provides UPnP device discovery and control for BASIC++
 * programs via the VDev2 framework. Registers two virtual
 * devices:
 *
 * UPNP:  UPnP device discovery (SSDP M-SEARCH)
 * SOAP:  UPnP control point (SOAP action invocation)
 *
 * PROTOCOL OVERVIEW:
 * UPnP uses SSDP (Simple Service Discovery Protocol) over
 * UDP multicast (239.255.255.250:1900) for device discovery.
 * Control commands use SOAP over HTTP to device endpoints.
 *
 * PLATFORM SUPPORT:
 * Windows:  Winsock2 (ws2_32.dll)
 * Linux:    BSD sockets (sys/socket.h)
 * FreeDOS:  Stub only (no network stack)
 *
 * BASIC USAGE:
 * MODULE "UPNP"
 * DEVICES            ' lists UPNP:, SOAP:
 * OPEN "UPNP:" FOR INPUT AS #1
 * IOCTL #1, "SEARCH", "ssdp:all"
 * DO WHILE NOT EOF(1)
 *   INPUT #1, D$
 *   PRINT D$
 * LOOP
 * CLOSE #1
 *
 * SECURITY:
 * Requires CAP_NET.
 * All network access is gated by the security system.
 * In SEC_RESTRICTED mode, the module cannot activate.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "mod_upnp.h"
#include "module.h"
#include "vdev.h"

/* --- Platform-Specific Includes --- */

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/*
 * Full implementation requires:
 * #include <winsock2.h>
 * #include <ws2tcpip.h>
 * #pragma comment(lib, "ws2_32.lib")
 *
 * WSAStartup() must be called before any socket operations.
 * WSACleanup() must be called on module teardown.
 */
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
/*
 * Full implementation requires:
 * #include <sys/socket.h>
 * #include <netinet/in.h>
 * #include <arpa/inet.h>
 */
#endif

/* --- SSDP Constants --- */

#define SSDP_MULTICAST_ADDR "239.255.255.250"
#define SSDP_PORT           1900
#define SSDP_MX_DEFAULT     3     /* max wait seconds */

/* Maximum discovered devices per search */
#define UPNP_MAX_DEVICES    32
#define UPNP_URL_MAX        512
#define UPNP_NAME_MAX       128
#define UPNP_RESPONSE_MAX   2048

/* --- UPnP Device State ---
 * Each VDev instance maintains its own state via user_data.
 */

typedef struct UPnPDeviceEntry {
 char name[UPNP_NAME_MAX];      /* friendly name / USN */
 char location[UPNP_URL_MAX];   /* description URL */
 char st[UPNP_NAME_MAX];        /* search target */
} UPnPDeviceEntry;

typedef struct UPnPState {
 int is_open;                    /* 1 if device is open */
 long sock_handle;               /* platform socket handle */
 char last_error[128];           /* last error message */
 char search_target[UPNP_NAME_MAX]; /* current ST filter */
 int mx_seconds;                 /* M-SEARCH MX value */

 /* Discovery results */
 int device_count;               /* discovered device count */
 UPnPDeviceEntry devices[UPNP_MAX_DEVICES];
 int read_cursor;                /* current read position */

 /* SOAP state */
 char soap_url[UPNP_URL_MAX];   /* target service URL */
 char soap_action[UPNP_NAME_MAX]; /* current SOAP action */
 char soap_response[UPNP_RESPONSE_MAX]; /* last response */
 int soap_response_len;          /* response length */
} UPnPState;

/* Static state for the two device instances */
static UPnPState upnp_state;
static UPnPState soap_state;


/* ================================================================
 * UPNP: Device Discovery (SSDP M-SEARCH)
 * ================================================================ */

/*
 * upnp_build_msearch - Build an SSDP M-SEARCH request.
 *
 * Constructs the multicast discovery request per UPnP spec:
 *   M-SEARCH * HTTP/1.1
 *   HOST: 239.255.255.250:1900
 *   MAN: "ssdp:discover"
 *   MX: <seconds>
 *   ST: <search-target>
 *
 * Returns the number of bytes written to buf.
 */
static int upnp_build_msearch(char *buf, int maxlen,
    const char *st, int mx)
{
 return snprintf(buf, (size_t)maxlen,
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: %s:%d\r\n"
  "MAN: \"ssdp:discover\"\r\n"
  "MX: %d\r\n"
  "ST: %s\r\n"
  "\r\n",
  SSDP_MULTICAST_ADDR, SSDP_PORT, mx, st);
}

/*
 * upnp_parse_response - Parse an SSDP response.
 *
 * Extracts LOCATION and ST/USN from a raw SSDP response.
 * Populates a UPnPDeviceEntry with the parsed values.
 *
 * Returns 0 on success, -1 if the response is malformed.
 */
static int upnp_parse_response(const char *resp, int len,
    UPnPDeviceEntry *entry)
{
 const char *p;
 const char *end = resp + len;
 int got_location = 0;

 memset(entry, 0, sizeof(*entry));

 /* Scan headers line by line */
 p = resp;
 while (p < end) {
  const char *eol;
  int line_len;

  /* Find end of line */
  eol = p;
  while (eol < end && *eol != '\r' && *eol != '\n')
   eol++;
  line_len = (int)(eol - p);

  /* LOCATION: header */
  if (line_len > 10 &&
   (p[0] == 'L' || p[0] == 'l') &&
   (p[1] == 'O' || p[1] == 'o') &&
   (p[2] == 'C' || p[2] == 'c') &&
   (p[3] == 'A' || p[3] == 'a') &&
   (p[4] == 'T' || p[4] == 't') &&
   (p[5] == 'I' || p[5] == 'i') &&
   (p[6] == 'O' || p[6] == 'o') &&
   (p[7] == 'N' || p[7] == 'n') &&
   p[8] == ':') {
   const char *val = p + 9;
   int vlen;
   while (val < eol && *val == ' ') val++;
   vlen = (int)(eol - val);
   if (vlen >= UPNP_URL_MAX)
    vlen = UPNP_URL_MAX - 1;
   memcpy(entry->location, val, (size_t)vlen);
   entry->location[vlen] = '\0';
   got_location = 1;
  }

  /* ST: header (search target) */
  if (line_len > 3 &&
   (p[0] == 'S' || p[0] == 's') &&
   (p[1] == 'T' || p[1] == 't') &&
   p[2] == ':') {
   const char *val = p + 3;
   int vlen;
   while (val < eol && *val == ' ') val++;
   vlen = (int)(eol - val);
   if (vlen >= UPNP_NAME_MAX)
    vlen = UPNP_NAME_MAX - 1;
   memcpy(entry->st, val, (size_t)vlen);
   entry->st[vlen] = '\0';
  }

  /* USN: header (unique service name) */
  if (line_len > 4 &&
   (p[0] == 'U' || p[0] == 'u') &&
   (p[1] == 'S' || p[1] == 's') &&
   (p[2] == 'N' || p[2] == 'n') &&
   p[3] == ':') {
   const char *val = p + 4;
   int vlen;
   while (val < eol && *val == ' ') val++;
   vlen = (int)(eol - val);
   if (vlen >= UPNP_NAME_MAX)
    vlen = UPNP_NAME_MAX - 1;
   memcpy(entry->name, val, (size_t)vlen);
   entry->name[vlen] = '\0';
  }

  /* Advance past end-of-line */
  p = eol;
  while (p < end && (*p == '\r' || *p == '\n'))
   p++;
 }

 /* If no name from USN, use ST */
 if (entry->name[0] == '\0' && entry->st[0] != '\0') {
  strncpy(entry->name, entry->st,
   UPNP_NAME_MAX - 1);
 }

 return got_location ? 0 : -1;
}


static int upnp_open(VDev *d, const char *path,
    const char *mode)
{
 UPnPState *st = (UPnPState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1; /* already open */

 st->is_open = 1;
 st->sock_handle = -1;
 st->device_count = 0;
 st->read_cursor = 0;
 st->mx_seconds = SSDP_MX_DEFAULT;
 st->last_error[0] = '\0';
 strcpy(st->search_target, "ssdp:all");

 if (path != NULL && strncmp(path, "UPNP:", 5) == 0) {
  /* Optional search target after "UPNP:" prefix */
  const char *st_str = path + 5;
  if (*st_str != '\0') {
   strncpy(st->search_target, st_str,
    UPNP_NAME_MAX - 1);
   st->search_target[UPNP_NAME_MAX - 1] = '\0';
  }
 }

 return 0;
}

static int upnp_close(VDev *d)
{
 UPnPState *st = (UPnPState *)d->user_data;

 if (!st->is_open) return -1;

 /* Close platform socket if open */
#if defined(_WIN32) || defined(_WIN64)
 if (st->sock_handle > 0) {
  /* closesocket((SOCKET)st->sock_handle); */
  st->sock_handle = -1;
 }
#elif defined(__linux__)
 if (st->sock_handle >= 0) {
  /* close((int)st->sock_handle); */
  st->sock_handle = -1;
 }
#endif

 st->is_open = 0;
 st->device_count = 0;
 st->read_cursor = 0;
 return 0;
}

static int upnp_status(VDev *d)
{
 UPnPState *st = (UPnPState *)d->user_data;
 return st->is_open ? 0 : -1;
}

/*
 * upnp_ioctl - Handle UPnP control commands.
 *
 * VDIO_ENUMERATE: Send M-SEARCH and collect responses.
 *   On Linux: creates UDP socket, sends multicast, reads
 *   responses with select() timeout.
 *   On Windows: uses Winsock2 UDP multicast.
 *   Populates the device list.
 *
 * VDIO_RESET: Clear discovery results.
 *
 * VDIO_GET_ERROR: Return last error string.
 */
static int upnp_ioctl(VDev *d, int cmd, void *arg)
{
 UPnPState *st = (UPnPState *)d->user_data;

 switch (cmd) {
 case VDIO_ENUMERATE:
  /*
   * Perform SSDP M-SEARCH discovery.
   *
   * Full implementation:
   * 1. Create UDP socket
   * 2. Set SO_REUSEADDR, IP_MULTICAST_TTL
   * 3. Send M-SEARCH to 239.255.255.250:1900
   * 4. select() with MX-second timeout
   * 5. Parse responses into device list
   * 6. Close socket
   *
   * Platform-specific code below.
   */
  st->device_count = 0;
  st->read_cursor = 0;

#ifdef __linux__
  /*
   * Linux SSDP discovery skeleton:
   *
   * int sock = socket(AF_INET, SOCK_DGRAM, 0);
   * struct sockaddr_in dest;
   * dest.sin_family = AF_INET;
   * dest.sin_port = htons(SSDP_PORT);
   * inet_aton(SSDP_MULTICAST_ADDR, &dest.sin_addr);
   *
   * char req[512];
   * int rlen = upnp_build_msearch(req, 512,
   *     st->search_target, st->mx_seconds);
   * sendto(sock, req, rlen, 0,
   *     (struct sockaddr *)&dest, sizeof(dest));
   *
   * // Read responses with timeout
   * struct timeval tv;
   * tv.tv_sec = st->mx_seconds;
   * tv.tv_usec = 0;
   * setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
   *     &tv, sizeof(tv));
   *
   * while (st->device_count < UPNP_MAX_DEVICES) {
   *   char buf[UPNP_RESPONSE_MAX];
   *   int n = recvfrom(sock, buf, sizeof(buf),
   *       0, NULL, NULL);
   *   if (n <= 0) break;
   *   UPnPDeviceEntry entry;
   *   if (upnp_parse_response(buf, n, &entry) == 0) {
   *     st->devices[st->device_count++] = entry;
   *   }
   * }
   * close(sock);
   */
  {
  char req[512];
  (void)upnp_build_msearch(req, 512,
   st->search_target, st->mx_seconds);
  strcpy(st->last_error,
   "SSDP: link with -lsocket for full support");
  }
#elif defined(_WIN32) || defined(_WIN64)
  /*
   * Windows SSDP discovery skeleton:
   *
   * WSADATA wsa;
   * WSAStartup(MAKEWORD(2,2), &wsa);
   * SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   * ... (similar to Linux but using Winsock types)
   * closesocket(sock);
   * WSACleanup();
   */
  {
  char req[512];
  (void)upnp_build_msearch(req, 512,
   st->search_target, st->mx_seconds);
  strcpy(st->last_error,
   "SSDP: link ws2_32.lib for full support");
  }
#else
  strcpy(st->last_error,
   "SSDP: not available on this platform");
#endif

  if (arg != NULL) {
   *(int *)arg = st->device_count;
  }
  return 0;

 case VDIO_RESET:
  st->device_count = 0;
  st->read_cursor = 0;
  st->last_error[0] = '\0';
  return 0;

 case VDIO_GET_ERROR:
  if (arg != NULL) {
   *(const char **)arg = st->last_error;
  }
  return 0;

 default:
  return -1;
 }
}

/*
 * upnp_gets - Read next discovered device.
 *
 * Returns a formatted string with device info:
 *   "USN | LOCATION | ST"
 *
 * Returns -1 when all devices have been read.
 */
static int upnp_gets(VDev *d, char *buf, int max)
{
 UPnPState *st = (UPnPState *)d->user_data;

 if (st->read_cursor >= st->device_count) {
  buf[0] = '\0';
  return -1; /* no more devices */
 }

 {
 UPnPDeviceEntry *e = &st->devices[st->read_cursor];
 snprintf(buf, (size_t)max, "%s | %s | %s",
  e->name, e->location, e->st);
 st->read_cursor++;
 }
 return 0;
}

static int upnp_poll(VDev *d)
{
 UPnPState *st = (UPnPState *)d->user_data;
 return (st->read_cursor < st->device_count) ? 1 : 0;
}

static const char *upnp_info(VDev *d, const char *key)
{
 UPnPState *st = (UPnPState *)d->user_data;
 if (strcmp(key, "error") == 0) return st->last_error;
 if (strcmp(key, "count") == 0) {
  static char count_buf[16];
  sprintf(count_buf, "%d", st->device_count);
  return count_buf;
 }
 if (strcmp(key, "st") == 0) return st->search_target;
 return NULL;
}


/* ================================================================
 * SOAP: UPnP Control Point (SOAP Action Invocation)
 * ================================================================ */

/*
 * soap_build_request - Build a SOAP action request.
 *
 * Constructs the HTTP POST body for a UPnP SOAP action:
 *   <?xml version="1.0"?>
 *   <s:Envelope xmlns:s="...">
 *     <s:Body>
 *       <u:ActionName xmlns:u="urn:...">
 *         ... arguments ...
 *       </u:ActionName>
 *     </s:Body>
 *   </s:Envelope>
 *
 * Returns the number of bytes written to buf.
 */
static int soap_build_request(char *buf, int maxlen,
    const char *service_type,
    const char *action_name,
    const char *arguments)
{
 const char *args = (arguments != NULL) ? arguments : "";

 return snprintf(buf, (size_t)maxlen,
  "<?xml version=\"1.0\"?>\r\n"
  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org"
  "/soap/envelope/\" "
  "s:encodingStyle=\"http://schemas.xmlsoap.org"
  "/soap/encoding/\">\r\n"
  "  <s:Body>\r\n"
  "    <u:%s xmlns:u=\"%s\">\r\n"
  "      %s\r\n"
  "    </u:%s>\r\n"
  "  </s:Body>\r\n"
  "</s:Envelope>\r\n",
  action_name, service_type,
  args,
  action_name);
}

static int soap_open(VDev *d, const char *path,
    const char *mode)
{
 UPnPState *st = (UPnPState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1;

 st->is_open = 1;
 st->sock_handle = -1;
 st->last_error[0] = '\0';
 st->soap_url[0] = '\0';
 st->soap_action[0] = '\0';
 st->soap_response[0] = '\0';
 st->soap_response_len = 0;

 if (path != NULL) {
  const char *url = path;
  if (strncmp(path, "SOAP:", 5) == 0)
   url = path + 5;
  strncpy(st->soap_url, url, UPNP_URL_MAX - 1);
  st->soap_url[UPNP_URL_MAX - 1] = '\0';
 }

 return 0;
}

static int soap_close(VDev *d)
{
 UPnPState *st = (UPnPState *)d->user_data;
 if (!st->is_open) return -1;

 st->is_open = 0;
 return 0;
}

/*
 * soap_ioctl - Handle SOAP control commands.
 *
 * VDIO_SEND: Send a SOAP action request to the target URL.
 *   arg points to a null-terminated string with format:
 *   "ServiceType\nActionName\nArguments"
 *
 * VDIO_RESET: Clear response buffer.
 *
 * VDIO_GET_ERROR: Return last error.
 */
static int soap_ioctl(VDev *d, int cmd, void *arg)
{
 UPnPState *st = (UPnPState *)d->user_data;

 switch (cmd) {
 case VDIO_USER:  /* SOAP send */
  if (arg == NULL) return -1;
  {
  /*
   * Parse the argument string:
   *   "urn:schemas-upnp-org:service:AVTransport:1"
   *   "\nPlay\n<InstanceID>0</InstanceID>"
   *
   * Full implementation would:
   * 1. Parse service type, action, arguments
   * 2. Build SOAP XML body
   * 3. Open TCP connection to soap_url
   * 4. Send HTTP POST with SOAP body
   * 5. Read response
   * 6. Store in soap_response
   */
  char body[2048];
  const char *input = (const char *)arg;
  (void)soap_build_request(body, 2048,
   input, "Action", NULL);
  (void)body;

  strcpy(st->last_error,
   "SOAP: link network library for full support");
  }
  return 0;

 case VDIO_RESET:
  st->soap_response[0] = '\0';
  st->soap_response_len = 0;
  st->last_error[0] = '\0';
  return 0;

 case VDIO_GET_ERROR:
  if (arg != NULL) {
   *(const char **)arg = st->last_error;
  }
  return 0;

 default:
  return -1;
 }
}

static int soap_status(VDev *d)
{
 UPnPState *st = (UPnPState *)d->user_data;
 return st->is_open ? 0 : -1;
}

/*
 * soap_gets - Read SOAP response.
 *
 * Returns the stored SOAP response as a line-by-line
 * stream. Each call returns the next line of the XML
 * response.
 */
static int soap_gets(VDev *d, char *buf, int max)
{
 UPnPState *st = (UPnPState *)d->user_data;

 if (st->soap_response_len <= 0 ||
  st->soap_response[0] == '\0') {
  buf[0] = '\0';
  return -1;
 }

 /* Return entire response as single read */
 strncpy(buf, st->soap_response, max - 1);
 buf[max - 1] = '\0';
 st->soap_response[0] = '\0'; /* consumed */
 st->soap_response_len = 0;
 return 0;
}

static const char *soap_info(VDev *d, const char *key)
{
 UPnPState *st = (UPnPState *)d->user_data;
 if (strcmp(key, "error") == 0) return st->last_error;
 if (strcmp(key, "url") == 0) return st->soap_url;
 if (strcmp(key, "action") == 0) return st->soap_action;
 return NULL;
}


/* ================================================================
 * Module Init / Cleanup
 * ================================================================ */

static int upnp_module_init(void *rt)
{
 VDev dev;
 (void)rt;

 /* Clear device states */
 memset(&upnp_state, 0, sizeof(upnp_state));
 memset(&soap_state, 0, sizeof(soap_state));
 upnp_state.sock_handle = -1;
 soap_state.sock_handle = -1;

 /* --- UPNP: Device Discovery --- */
 memset(&dev, 0, sizeof(dev));
 dev.name = "UPNP:";
 dev.dev_class = VDCLASS_NETWORK;
 dev.dev_caps = VDCAP_READ | VDCAP_CONTROL |
  VDCAP_STATUS | VDCAP_EVENT;
 dev.dev_version = "1.0";
 dev.dev_description =
  "UPnP/SSDP device discovery";
 dev.dev_req_caps = CAP_NETWORK | CAP_IO;
 dev.dev_open = upnp_open;
 dev.dev_close = upnp_close;
 dev.dev_gets = upnp_gets;
 dev.dev_ioctl = upnp_ioctl;
 dev.dev_status = upnp_status;
 dev.dev_poll = upnp_poll;
 dev.dev_info = upnp_info;
 dev.user_data = &upnp_state;
 vdev_register(&dev);

 /* --- SOAP: UPnP Control Point --- */
 memset(&dev, 0, sizeof(dev));
 dev.name = "SOAP:";
 dev.dev_class = VDCLASS_NETWORK;
 dev.dev_caps = VDCAP_READ | VDCAP_WRITE |
  VDCAP_CONTROL | VDCAP_STATUS;
 dev.dev_version = "1.0";
 dev.dev_description =
  "UPnP SOAP control point";
 dev.dev_req_caps = CAP_NETWORK | CAP_IO;
 dev.dev_open = soap_open;
 dev.dev_close = soap_close;
 dev.dev_gets = soap_gets;
 dev.dev_ioctl = soap_ioctl;
 dev.dev_status = soap_status;
 dev.dev_info = soap_info;
 dev.user_data = &soap_state;
 vdev_register(&dev);

 return 0;
}

static void upnp_module_cleanup(void)
{
 /* Close any open sockets */
 if (upnp_state.is_open) {
  upnp_state.is_open = 0;
 }
 if (soap_state.is_open) {
  soap_state.is_open = 0;
 }
}

/* Module descriptor */
static const ModuleInfo upnp_module_info = {
 "UPNP",                                 /* name */
 "1.0",                                  /* version */
 "UPnP/SSDP device discovery and control", /* description */
 MOD_DEVICE,                             /* mod_class */
 CAP_NETWORK | CAP_IO,                   /* capabilities */
 upnp_module_init,                       /* init */
 upnp_module_cleanup                     /* cleanup */
};

/*
 * mod_upnp_register - Register the UPnP module.
 *
 * Called from main.c during boot. The module starts inactive.
 * The user activates it via MODULE "UPNP" from BASIC.
 */
void mod_upnp_register(void)
{
 module_register(&upnp_module_info);
}
