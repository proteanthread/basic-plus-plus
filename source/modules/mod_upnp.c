/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_upnp.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - mod_upnp.c
 // ---
 //
 // UPnP/SSDP Device Discovery Module.
 //
 // PURPOSE:
 // Provides UPnP device discovery and control for BASIC++
 // programs via the VDev2 framework. Registers two virtual
 // devices:
 //
 // UPNP:  UPnP device discovery (SSDP M-SEARCH)
 // SOAP:  UPnP control point (SOAP action invocation)
 //
 // PROTOCOL OVERVIEW:
 // UPnP uses SSDP (Simple Service Discovery Protocol) over
 // UDP multicast (239.255.255.250:1900) for device discovery.
 // Control commands use SOAP over HTTP to device endpoints.
 //
 // PLATFORM SUPPORT:
 // Windows:  Winsock2 (ws2_32.dll)
 // Linux:    BSD sockets (sys/socket.h)
 // FreeDOS:  Stub only (no network stack)
 //
 // BASIC USAGE:
 // MODULE "UPNP"
 // DEVICES            ' lists UPNP:, SOAP:
 // OPEN "UPNP:" FOR INPUT AS #1
 // IOCTL #1, "SEARCH", "ssdp:all"
 // DO WHILE NOT EOF(1)
 //   INPUT #1, D$
 //   PRINT D$
 // LOOP
 // CLOSE #1
 //
 // SECURITY:
 // Requires CAP_NET.
 // All network access is gated by the security system.
 // In SEC_RESTRICTED mode, the module cannot activate.
 //
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "mod_upnp.h"
#include "module.h"
#include "vdev.h"

// --- Platform-Specific Socket Abstraction ---

#if defined(_WIN32) || defined(_WIN64)
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #pragma comment(lib, "ws2_32.lib")
 typedef int socklen_t;
 #define UPNP_CLOSESOCKET closesocket
 #define UPNP_INVALID_SOCKET INVALID_SOCKET
 static int upnp_wsa_initialized = 0;
#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
 #define UPNP_NO_NETWORKING 1
 #define UPNP_CLOSESOCKET(s) ((void)(s))
 #define UPNP_INVALID_SOCKET (-1)
 typedef int SOCKET;
#else
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/select.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #define UPNP_CLOSESOCKET close
 #define UPNP_INVALID_SOCKET (-1)
 typedef int SOCKET;
#endif

// --- SSDP Constants ---

#define SSDP_MULTICAST_ADDR "239.255.255.250"
#define SSDP_PORT           1900
#define SSDP_MX_DEFAULT     3 // max wait seconds

// Maximum discovered devices per search
#define UPNP_MAX_DEVICES    32
#define UPNP_URL_MAX        512
#define UPNP_NAME_MAX       128
#define UPNP_RESPONSE_MAX   2048

// --- UPnP Device State ---
 // Each VDev instance maintains its own state via user_data.

typedef struct UPnPDeviceEntry {
 char name[UPNP_NAME_MAX]; // friendly name / USN
 char location[UPNP_URL_MAX]; // description URL
 char st[UPNP_NAME_MAX]; // search target
} UPnPDeviceEntry;

typedef struct UPnPState {
 int is_open; // 1 if device is open
 SOCKET sock_handle; // platform socket handle
 char last_error[128]; // last error message
 char search_target[UPNP_NAME_MAX]; // current ST filter
 int mx_seconds; // M-SEARCH MX value

 // Discovery results
 int device_count; // discovered device count
 UPnPDeviceEntry devices[UPNP_MAX_DEVICES];
 int read_cursor; // current read position

 // SOAP state
 char soap_url[UPNP_URL_MAX]; // target service URL
 char soap_action[UPNP_NAME_MAX]; // current SOAP action
 char soap_response[UPNP_RESPONSE_MAX]; // last response
 int soap_response_len; // response length
} UPnPState;

// Static state for the two device instances
static UPnPState upnp_state;
static UPnPState soap_state;


// ================================================================
 // UPNP: Device Discovery (SSDP M-SEARCH)
 // ================================================================ 

 // upnp_build_msearch - Build an SSDP M-SEARCH request.
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

 // upnp_parse_response - Parse an SSDP response.
static int upnp_parse_response(const char *resp, int len,
    UPnPDeviceEntry *entry)
{
 const char *p;
 const char *end = resp + len;
 int got_location = 0;

 memset(entry, 0, sizeof(*entry));

 p = resp;
 while (p < end) {
  const char *eol;
  int line_len;

  eol = p;
  while (eol < end && *eol != '\r' && *eol != '\n')
   eol++;
  line_len = (int)(eol - p);

  // LOCATION: header
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

  // ST: header
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

  // USN: header
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

  p = eol;
  while (p < end && (*p == '\r' || *p == '\n'))
   p++;
 }

 if (entry->name[0] == '\0' && entry->st[0] != '\0') {
  snprintf(entry->name, UPNP_NAME_MAX, "%s", entry->st);
 }

 return got_location ? 0 : -1;
}


static int upnp_open(VDev *d, const char *path,
    const char *mode)
{
 UPnPState *st = (UPnPState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1;

 st->is_open = 1;
 st->sock_handle = (SOCKET)UPNP_INVALID_SOCKET;
 st->device_count = 0;
 st->read_cursor = 0;
 st->mx_seconds = SSDP_MX_DEFAULT;
 st->last_error[0] = '\0';
 strcpy(st->search_target, "ssdp:all");

 if (path != NULL && strncmp(path, "UPNP:", 5) == 0) {
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

#ifndef UPNP_NO_NETWORKING
 if (st->sock_handle != (SOCKET)UPNP_INVALID_SOCKET) {
  UPNP_CLOSESOCKET(st->sock_handle);
  st->sock_handle = (SOCKET)UPNP_INVALID_SOCKET;
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

 // upnp_ioctl - Handle UPnP control commands.
 //
 // VDIO_ENUMERATE: Send M-SEARCH and collect responses.
 // VDIO_RESET: Clear discovery results.
 // VDIO_GET_ERROR: Return last error string.
static int upnp_ioctl(VDev *d, int cmd, void *arg)
{
 UPnPState *st = (UPnPState *)d->user_data;

 switch (cmd) {
 case VDIO_ENUMERATE:
   // Perform SSDP M-SEARCH discovery.
   // Real socket implementation for Windows/Linux.
  st->device_count = 0;
  st->read_cursor = 0;
  st->last_error[0] = '\0';

#ifdef UPNP_NO_NETWORKING
  strcpy(st->last_error,
   "SSDP: not available on this platform");
#else
  {
  SOCKET sock;
  struct sockaddr_in dest;
  char req[512];
  int rlen;
  int ttl = 4;
  int reuse = 1;
#ifndef _WIN32
  struct timeval tv;
#else
  int timeout_ms;
#endif

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == (SOCKET)UPNP_INVALID_SOCKET) {
   strcpy(st->last_error,
    "SSDP: socket creation failed");
   break;
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
   (const char *)&reuse, sizeof(reuse));
  setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
   (const char *)&ttl, sizeof(ttl));

  // Set receive timeout
#ifdef _WIN32
  timeout_ms = st->mx_seconds * 1000;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
   (const char *)&timeout_ms, sizeof(timeout_ms));
#else
  tv.tv_sec = st->mx_seconds;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
   &tv, sizeof(tv));
#endif

  // Build and send M-SEARCH
  rlen = upnp_build_msearch(req, (int)sizeof(req),
   st->search_target, st->mx_seconds);

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(SSDP_PORT);
  inet_pton(AF_INET, SSDP_MULTICAST_ADDR,
   &dest.sin_addr);

  if (sendto(sock, req, rlen, 0,
   (struct sockaddr *)&dest,
   sizeof(dest)) < 0) {
   strcpy(st->last_error,
    "SSDP: send failed");
   UPNP_CLOSESOCKET(sock);
   break;
  }

  // Read responses until timeout
  while (st->device_count < UPNP_MAX_DEVICES) {
   char buf[UPNP_RESPONSE_MAX];
   int n;
   UPnPDeviceEntry entry;

   n = recvfrom(sock, buf, sizeof(buf) - 1,
    0, NULL, NULL);
   if (n <= 0) break;
   buf[n] = '\0';

   if (upnp_parse_response(buf, n, &entry) == 0) {
    st->devices[st->device_count++] = entry;
   }
  }

  UPNP_CLOSESOCKET(sock);
  }
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

 return 0;
}

 // upnp_gets - Read next discovered device.
static int upnp_gets(VDev *d, char *buf, int max)
{
 UPnPState *st = (UPnPState *)d->user_data;

 if (st->read_cursor >= st->device_count) {
  buf[0] = '\0';
  return -1;
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


// ================================================================
 // SOAP: UPnP Control Point (SOAP Action Invocation)
 // ================================================================ 

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
 st->sock_handle = (SOCKET)UPNP_INVALID_SOCKET;
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

static int soap_ioctl(VDev *d, int cmd, void *arg)
{
 UPnPState *st = (UPnPState *)d->user_data;

 switch (cmd) {
 case VDIO_USER: // SOAP send
  if (arg == NULL) return -1;
#ifdef UPNP_NO_NETWORKING
  strcpy(st->last_error,
   "SOAP: not available on this platform");
  return -1;
#else
  {
  char body[2048];
  const char *input = (const char *)arg;
  int body_len;
  char host[256];
  char path[256];
  int port = 80;
  const char *p;
  const char *pp;
  struct addrinfo hints, *res;
  SOCKET sock;
  char port_str[16];
  char request[4096];
  int req_len;
  int total_read = 0;

  // Build the SOAP XML body
  body_len = soap_build_request(body, (int)sizeof(body),
   input, "Action", NULL);
  if (body_len <= 0) {
   strcpy(st->last_error, "SOAP: bad request body");
   return -1;
  }

  // Parse soap_url: http://host:port/path
  host[0] = '\0';
  path[0] = '/'; path[1] = '\0';
  p = st->soap_url;

  // Skip http:// if present
  if (strncmp(p, "http://", 7) == 0) p += 7;
  else if (strncmp(p, "HTTP://", 7) == 0) p += 7;

  // Extract host
  pp = p;
  while (*pp && *pp != ':' && *pp != '/' && *pp != '\0')
   pp++;
  {
   int hlen = (int)(pp - p);
   if (hlen <= 0 || hlen >= (int)sizeof(host)) {
    strcpy(st->last_error, "SOAP: bad URL");
    return -1;
   }
   memcpy(host, p, (size_t)hlen);
   host[hlen] = '\0';
  }

  // Extract port if present
  if (*pp == ':') {
   pp++;
   port = atoi(pp);
   while (*pp >= '0' && *pp <= '9') pp++;
  }

  // Extract path
  if (*pp == '/') {
   strncpy(path, pp, sizeof(path) - 1);
   path[sizeof(path) - 1] = '\0';
  }

  // Connect via TCP
  sprintf(port_str, "%d", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if (getaddrinfo(host, port_str, &hints, &res) != 0) {
   strcpy(st->last_error, "SOAP: DNS failed");
   return -1;
  }

  sock = socket(res->ai_family, res->ai_socktype,
   res->ai_protocol);
  if (sock == (SOCKET)UPNP_INVALID_SOCKET) {
   freeaddrinfo(res);
   strcpy(st->last_error, "SOAP: socket failed");
   return -1;
  }

  if (connect(sock, res->ai_addr,
   (int)res->ai_addrlen) < 0) {
   UPNP_CLOSESOCKET(sock);
   freeaddrinfo(res);
   strcpy(st->last_error, "SOAP: connect refused");
   return -1;
  }
  freeaddrinfo(res);

  // Build HTTP POST request
  req_len = snprintf(request, sizeof(request),
   "POST %s HTTP/1.1\r\n"
   "Host: %s:%d\r\n"
   "Content-Type: text/xml; charset=utf-8\r\n"
   "SOAPAction: \"%s#Action\"\r\n"
   "Content-Length: %d\r\n"
   "Connection: close\r\n"
   "\r\n"
   "%s",
   path, host, port, input, body_len, body);

  // Send request
  if (send(sock, request, req_len, 0) < 0) {
   UPNP_CLOSESOCKET(sock);
   strcpy(st->last_error, "SOAP: send failed");
   return -1;
  }

  // Read response
  st->soap_response[0] = '\0';
  st->soap_response_len = 0;
  total_read = 0;
  while (total_read < UPNP_RESPONSE_MAX - 1) {
   int n = recv(sock, st->soap_response + total_read,
    UPNP_RESPONSE_MAX - 1 - total_read, 0);
   if (n <= 0) break;
   total_read += n;
  }
  st->soap_response[total_read] = '\0';
  st->soap_response_len = total_read;

  UPNP_CLOSESOCKET(sock);

  // Check for HTTP error
  if (total_read > 12 &&
   strncmp(st->soap_response, "HTTP/", 5) == 0) {
   const char *sp = strchr(st->soap_response, ' ');
   if (sp) {
    int status = atoi(sp + 1);
    if (status >= 200 && status < 300) {
     st->last_error[0] = '\0';
    } else {
     snprintf(st->last_error,
      sizeof(st->last_error),
      "SOAP: HTTP %d", status);
    }
   }
  }

  // Strip HTTP headers from response,
   // keep only the SOAP XML body 
  {
   char *body_start = strstr(st->soap_response,
    "\r\n\r\n");
   if (body_start) {
    body_start += 4;
    {
    int blen = total_read -
     (int)(body_start - st->soap_response);
    memmove(st->soap_response,
     body_start, (size_t)blen);
    st->soap_response[blen] = '\0';
    st->soap_response_len = blen;
    }
   }
  }
  }
  return 0;
#endif

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

static int soap_gets(VDev *d, char *buf, int max)
{
 UPnPState *st = (UPnPState *)d->user_data;

 if (st->soap_response_len <= 0 ||
  st->soap_response[0] == '\0') {
  buf[0] = '\0';
  return -1;
 }

 strncpy(buf, st->soap_response, max - 1);
 buf[max - 1] = '\0';
 st->soap_response[0] = '\0';
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


// ================================================================
 // Module Init / Cleanup
 // ================================================================ 

static int upnp_module_init(void *rt)
{
 VDev dev;
 (void)rt;

 // Clear device states
 memset(&upnp_state, 0, sizeof(upnp_state));
 memset(&soap_state, 0, sizeof(soap_state));
 upnp_state.sock_handle = (SOCKET)UPNP_INVALID_SOCKET;
 soap_state.sock_handle = (SOCKET)UPNP_INVALID_SOCKET;

#if defined(_WIN32) || defined(_WIN64)
 if (!upnp_wsa_initialized) {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
   return -1;
  upnp_wsa_initialized = 1;
 }
#endif

 // --- UPNP: Device Discovery ---
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

 // --- SOAP: UPnP Control Point ---
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
 // Close any open sockets
#ifndef UPNP_NO_NETWORKING
 if (upnp_state.is_open &&
     upnp_state.sock_handle !=
     (SOCKET)UPNP_INVALID_SOCKET) {
  UPNP_CLOSESOCKET(upnp_state.sock_handle);
  upnp_state.sock_handle =
   (SOCKET)UPNP_INVALID_SOCKET;
 }
 if (soap_state.is_open &&
     soap_state.sock_handle !=
     (SOCKET)UPNP_INVALID_SOCKET) {
  UPNP_CLOSESOCKET(soap_state.sock_handle);
  soap_state.sock_handle =
   (SOCKET)UPNP_INVALID_SOCKET;
 }
#endif
 upnp_state.is_open = 0;
 soap_state.is_open = 0;

#if defined(_WIN32) || defined(_WIN64)
 if (upnp_wsa_initialized) {
  WSACleanup();
  upnp_wsa_initialized = 0;
 }
#endif
}

// Module descriptor
static const ModuleInfo upnp_module_info = {
 "UPNP", // name
 "1.0", // version
 "UPnP/SSDP device discovery and control", // description
 MOD_DEVICE, // mod_class
 CAP_NETWORK | CAP_IO, // capabilities
 upnp_module_init, // init
 upnp_module_cleanup // cleanup
};

 // mod_upnp_register - Register the UPnP module.
void mod_upnp_register(void)
{
 module_register(&upnp_module_info);
}
