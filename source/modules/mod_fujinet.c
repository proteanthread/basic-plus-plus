 // ---
 // BASIC++ Interpreter - mod_fujinet.c
 // ---
 //
 // FujiNet Virtual Device Module -- Full Implementation.
 //
 // Three virtual devices that expose FujiNet hardware services
 // through the VDev2 interface:
 //
 //   N:      Network adapter (TCP/UDP/HTTP/TNFS/TELNET)
 //   FUJI:   Configuration (WiFi, slots, AppKey, directory)
 //   CLOCK:  Network time via NTP
 //
 // Desktop implementation uses Winsock2 (Windows) or BSD sockets
 // (POSIX) for the N: device. FUJI: uses local config files and
 // platform APIs. CLOCK: returns system time.
 //
 // When compiled for FujiNet hardware targets (Atari, Apple II,
 // C64, CoCo), replace the socket calls with fujinet-lib calls.
 //
//
// HOW TO EXTEND:
//   To add new functions to this module:
//   1. Add the function implementation in this file.
//   2. Register it in the module's init function using
//      module_register_function().
//   3. Update the module's header with the new declaration.
//
// TROUBLESHOOTING:
//   - Module not loading: check module_init() registration.
//   - Function not found: verify registration name matches
//     the BASIC keyword exactly (case-insensitive).
 // ---

// POSIX feature test macros -- needed for getaddrinfo/struct addrinfo
 // when compiling with -std=c90 on Linux/POSIX systems. 
#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200112L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
  #endif
  #ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "config.h"
#include "mod_fujinet.h"
#include "module.h"
#include "vdev.h"
#include "errors.h"

// Directory API for FUJI: IOCTLs
#ifdef _WIN32
  // Windows directory enumeration
  // windows.h already pulled in by winsock2.h below
  #define fn_popen  _popen
  #define fn_pclose _pclose
#elif !defined(__MSDOS__) && !defined(__DOS__)
  #include <dirent.h>
  #include <sys/stat.h>
  #define fn_popen  popen
  #define fn_pclose pclose
#else
  #define fn_popen(c,m)  ((void)(c),(void)(m),(FILE*)0)
  #define fn_pclose(f)   ((void)(f), -1)
#endif

// ================================================================
 // PLATFORM-SPECIFIC SOCKET ABSTRACTION
 // ================================================================ 

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h> // FindFirstFileA etc. for FUJI: dir browsing
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
  #define FN_INVALID_SOCKET INVALID_SOCKET
  #define FN_SOCKET_ERROR   SOCKET_ERROR
  #define fn_closesocket    closesocket
  #define fn_socket_errno   WSAGetLastError()
  static int wsa_initialized = 0;
#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
  // DOS stub -- no networking available
  #define FN_INVALID_SOCKET (-1)
  #define FN_SOCKET_ERROR   (-1)
  #define fn_closesocket(s) ((void)(s), -1)
  #define fn_socket_errno   0
  typedef int SOCKET;
  typedef int socklen_t;
  #define FN_NO_NETWORKING 1
  // Stub socket constants and functions for compilation
  #define AF_INET     2
  #define SOCK_STREAM 1
  #define SOCK_DGRAM  2
  #define IPPROTO_TCP 6
  #define IPPROTO_UDP 17
  #define MSG_PEEK    2
  #define send(s,b,l,f)    ((void)(s),(void)(b),(void)(l),(void)(f),-1)
  #define recv(s,b,l,f)    ((void)(s),(void)(b),(void)(l),(void)(f),-1)
  #define connect(s,a,l)   ((void)(s),(void)(a),(void)(l),-1)
  #define socket(d,t,p)    ((void)(d),(void)(t),(void)(p),-1)
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <errno.h>
  #define FN_INVALID_SOCKET (-1)
  #define FN_SOCKET_ERROR   (-1)
  #define fn_closesocket    close
  #define fn_socket_errno   errno
  typedef int SOCKET;
#endif

// ================================================================
 // MODULE STATE
 // ================================================================ 

static FnChannel fn_channels[FN_MAX_CHANNELS];
static FnFujiState fn_fuji;
static FnClockState fn_clock_state;

// VDev IDs assigned at registration
static int fn_net_vdev_id = -1;
static int fn_fuji_vdev_id = -1;
static int fn_clock_vdev_id = -1;

// ================================================================
 // INTERNAL HELPERS
 // ================================================================ 

 // Initialize the socket subsystem (Winsock on Windows).
static int fn_socket_init(void)
{
#ifdef FN_NO_NETWORKING
    return -1; // no networking on DOS
#elif defined(_WIN32)
    WSADATA wsa;
    if (!wsa_initialized) {
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return -1;
        wsa_initialized = 1;
    }
    return 0;
#else
    return 0;
#endif
}

 // Shut down the socket subsystem.
static void fn_socket_cleanup(void)
{
#ifdef FN_NO_NETWORKING
    // nothing to clean up on DOS
#elif defined(_WIN32)
    if (wsa_initialized) {
        WSACleanup();
        wsa_initialized = 0;
    }
#endif
}

 // Set a socket to non-blocking mode.
static int fn_set_nonblocking(int fd)
{
#ifdef FN_NO_NETWORKING
    (void)fd;
    return -1;
#elif defined(_WIN32)
    u_long mode = 1;
    return ioctlsocket((SOCKET)fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

 // Parse a FujiNet devicespec URL into protocol, host, port, path.
 //
 // Format: N:PROTO://host:port/path
 //         or just PROTO://host:port/path
 //
 // Returns 0 on success, -1 on parse failure.
static int fn_parse_devicespec(const char *spec,
    FnProto *proto, char *host, int host_max,
    int *port, char *path, int path_max)
{
    const char *p = spec;
    const char *proto_start;
    const char *proto_end;
    const char *host_start;
    const char *host_end;
    const char *port_start;
    char proto_str[16];
    int proto_len;
    int i;

    // Skip leading "N:" if present
    if ((p[0] == 'N' || p[0] == 'n') && p[1] == ':')
        p += 2;

    // Extract protocol (everything before "://")
    proto_start = p;
    proto_end = strstr(p, "://");
    if (proto_end == NULL) return -1;

    proto_len = (int)(proto_end - proto_start);
    if (proto_len <= 0 || proto_len > 15) return -1;

    for (i = 0; i < proto_len; i++)
        proto_str[i] = (char)toupper((unsigned char)proto_start[i]);
    proto_str[proto_len] = '\0';

    // Map protocol string to enum
    *proto = FN_PROTO_UNKNOWN;
    if (strcmp(proto_str, "TCP") == 0)
        *proto = FN_PROTO_TCP;
    else if (strcmp(proto_str, "UDP") == 0)
        *proto = FN_PROTO_UDP;
    else if (strcmp(proto_str, "HTTP") == 0)
        *proto = FN_PROTO_HTTP;
    else if (strcmp(proto_str, "HTTPS") == 0)
        *proto = FN_PROTO_HTTPS;
    else if (strcmp(proto_str, "TNFS") == 0)
        *proto = FN_PROTO_TNFS;
    else if (strcmp(proto_str, "FTP") == 0)
        *proto = FN_PROTO_FTP;
    else if (strcmp(proto_str, "TELNET") == 0)
        *proto = FN_PROTO_TELNET;
    else if (strcmp(proto_str, "SSH") == 0)
        *proto = FN_PROTO_SSH;

    if (*proto == FN_PROTO_UNKNOWN) return -1;

    // Skip past "://"
    p = proto_end + 3;

    // Extract host (up to ':' or '/' or end)
    host_start = p;
    host_end = p;
    while (*host_end && *host_end != ':' && *host_end != '/')
        host_end++;

    i = (int)(host_end - host_start);
    if (i <= 0 || i >= host_max) return -1;
    memcpy(host, host_start, i);
    host[i] = '\0';

    // Extract port (optional)
    *port = 0;
    if (*host_end == ':') {
        port_start = host_end + 1;
        *port = 0;
        while (*port_start >= '0' && *port_start <= '9') {
            *port = (*port * 10) + (*port_start - '0');
            port_start++;
        }
        p = port_start;
    } else {
        p = host_end;
    }

    // Default ports by protocol
    if (*port == 0) {
        switch (*proto) {
        case FN_PROTO_HTTP:   *port = 80; break;
        case FN_PROTO_HTTPS:  *port = 443; break;
        case FN_PROTO_FTP:    *port = 21; break;
        case FN_PROTO_TELNET: *port = 23; break;
        case FN_PROTO_SSH:    *port = 22; break;
        case FN_PROTO_TNFS:   *port = 16384; break;
        default: break;
        }
    }

    // Extract path (everything after host:port)
    if (*p == '/') {
        i = (int)strlen(p);
        if (i >= path_max) i = path_max - 1;
        memcpy(path, p, i);
        path[i] = '\0';
    } else {
        path[0] = '/';
        path[1] = '\0';
    }

    return 0;
}

 // Find a free channel slot.
static int fn_alloc_channel(void)
{
    int i;
    for (i = 0; i < FN_MAX_CHANNELS; i++) {
        if (!fn_channels[i].in_use)
            return i;
    }
    return -1;
}

 // Resolve hostname and connect a TCP socket.
static int fn_tcp_connect(const char *host, int port)
{
#ifdef FN_NO_NETWORKING
    (void)host; (void)port;
    return -1;
#else
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct addrinfo *rp;
    char port_str[8];
    int fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    sprintf(port_str, "%d", port);

    if (getaddrinfo(host, port_str, &hints, &result) != 0)
        return -1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fd = (int)socket(rp->ai_family, rp->ai_socktype,
                         rp->ai_protocol);
        if (fd < 0) continue;

        if (connect(fd, rp->ai_addr,
                    (int)rp->ai_addrlen) == 0) {
            break; // connected
        }
        fn_closesocket(fd);
        fd = -1;
    }

    freeaddrinfo(result);
    return fd;
#endif
}

 // Create and bind a UDP socket.
static int fn_udp_create(const char *host, int port)
{
#ifdef FN_NO_NETWORKING
    (void)host; (void)port;
    return -1;
#else
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    char port_str[8];
    int fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    sprintf(port_str, "%d", port);

    if (getaddrinfo(host, port_str, &hints, &result) != 0)
        return -1;

    fd = (int)socket(result->ai_family, result->ai_socktype,
                     result->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(result);
        return -1;
    }

    // For UDP, "connect" sets the default destination
    if (connect(fd, result->ai_addr,
                (int)result->ai_addrlen) != 0) {
        fn_closesocket(fd);
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);
    return fd;
#endif
}

// ================================================================
 // HTTP CLIENT (simplified, desktop only)
 //
 // Builds an HTTP/1.1 request, sends it over a TCP socket,
 // and captures the response headers + body into the channel.
 // ================================================================ 

 // Build and send an HTTP GET request, read the full response.
static int fn_http_request(FnChannel *ch, const char *method)
{
    char request[1024];
    char response[FN_HTTP_BODY_MAX + FN_HTTP_HEADER_MAX];
    int total = 0;
    int n;
    char *body_start;
    char *status_line;
    int header_len;

    // Build HTTP/1.1 request
    sprintf(request,
        "%s %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: BASIC++/1.0 (FujiNet)\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n"
        "%s"
        "\r\n",
        method, ch->path, ch->host,
        ch->http_headers[0] ? ch->http_headers : "");

    // Send request
    n = send(ch->sock_fd, request, (int)strlen(request), 0);
    if (n <= 0) return FN_ERR_IO_ERROR;

    // If POST/PUT, send body
    if (ch->http_body && ch->http_body_len > 0 &&
        (strcmp(method, "POST") == 0 ||
         strcmp(method, "PUT") == 0)) {
        n = send(ch->sock_fd, ch->http_body,
                 ch->http_body_len, 0);
        if (n <= 0) return FN_ERR_IO_ERROR;
    }

    // Read response in chunks
    memset(response, 0, sizeof(response));
    while (total < (int)sizeof(response) - 1) {
        n = recv(ch->sock_fd, response + total,
                 (int)sizeof(response) - 1 - total, 0);
        if (n <= 0) break;
        total += n;
    }
    response[total] = '\0';

    // Parse status code from first line
    status_line = response;
    ch->http_status_code = 0;
    if (strncmp(status_line, "HTTP/", 5) == 0) {
        char *sp = strchr(status_line, ' ');
        if (sp) ch->http_status_code = atoi(sp + 1);
    }

    // Split headers from body at \r\n\r\n
    body_start = strstr(response, "\r\n\r\n");
    if (body_start) {
        header_len = (int)(body_start - response);
        if (header_len > FN_HTTP_HEADER_MAX - 1)
            header_len = FN_HTTP_HEADER_MAX - 1;
        memcpy(ch->http_headers, response, header_len);
        ch->http_headers[header_len] = '\0';

        body_start += 4; // skip \r\n\r\n
        ch->http_body_len = total - (int)(body_start - response);
        if (ch->http_body_len > 0) {
            ch->http_body = (char *)malloc(
                ch->http_body_len + 1);
            if (ch->http_body) {
                memcpy(ch->http_body, body_start,
                       ch->http_body_len);
                ch->http_body[ch->http_body_len] = '\0';
            }
        }
    }

    ch->http_body_pos = 0;
    ch->bytes_waiting = (unsigned int)ch->http_body_len;
    ch->connected = 1;
    return FN_ERR_OK;
}

// ================================================================
 // TNFS CLIENT (Trivial Network File System)
 //
 // TNFS uses UDP packets with a simple header:
 //   [session_hi][session_lo][seq][cmd][payload...]
 //
 // This implements mount, open, read, write, close, unmount.
 // ================================================================ 

 // Send a TNFS packet and receive the response.
 // Returns response payload length, or -1 on error.
static int fn_tnfs_transaction(FnChannel *ch,
    unsigned char cmd,
    const unsigned char *payload, int payload_len,
    unsigned char *resp, int resp_max)
{
    unsigned char pkt[FN_TNFS_PACKET_MAX];
    int pkt_len;
    int n;

    // Build TNFS packet header
    pkt[0] = ch->tnfs_session[0];
    pkt[1] = ch->tnfs_session[1];
    pkt[2] = ch->tnfs_seq++;
    pkt[3] = cmd;
    pkt_len = 4;

    if (payload && payload_len > 0) {
        if (payload_len > FN_TNFS_PACKET_MAX - 4)
            payload_len = FN_TNFS_PACKET_MAX - 4;
        memcpy(pkt + 4, payload, payload_len);
        pkt_len += payload_len;
    }

    // Send via UDP
    n = send(ch->sock_fd, (char *)pkt, pkt_len, 0);
    if (n <= 0) return -1;

    // Receive response
    n = recv(ch->sock_fd, (char *)resp, resp_max, 0);
    if (n < 4) return -1;

    // Check for error in response (byte 4 = return code)
    if (n > 4 && resp[4] != 0) {
        ch->last_error = resp[4];
        return -1;
    }

    return n;
}

 // Mount a TNFS share. Establishes the session.
 // TNFS CMD 0x00 = MOUNT
static int fn_tnfs_mount(FnChannel *ch)
{
    unsigned char payload[256];
    unsigned char resp[FN_TNFS_PACKET_MAX];
    int plen = 0;
    int n;

    // Version 1.2, no auth
    payload[plen++] = 0x01; // version major
    payload[plen++] = 0x02; // version minor

    // Mount path (null-terminated)
    if (ch->path[0]) {
        int pathlen = (int)strlen(ch->path);
        if (pathlen > 200) pathlen = 200;
        memcpy(payload + plen, ch->path, pathlen);
        plen += pathlen;
    }
    payload[plen++] = 0x00;

    // No username or password
    payload[plen++] = 0x00;
    payload[plen++] = 0x00;

    // Clear session for mount request
    ch->tnfs_session[0] = 0;
    ch->tnfs_session[1] = 0;
    ch->tnfs_seq = 0;

    n = fn_tnfs_transaction(ch, 0x00, payload, plen,
                            resp, sizeof(resp));
    if (n < 6) return FN_ERR_IO_ERROR;

    // Response: [session_hi][session_lo][seq][cmd][err][ver_lo][ver_hi]
    ch->tnfs_session[0] = resp[0];
    ch->tnfs_session[1] = resp[1];
    return FN_ERR_OK;
}

 // Open a file on a TNFS share.
 // TNFS CMD 0x29 = OPEN
static int fn_tnfs_open(FnChannel *ch, const char *filename, int mode)
{
    unsigned char payload[256];
    unsigned char resp[FN_TNFS_PACKET_MAX];
    int plen = 0;
    int flen;
    int n;

    // Open flags: 0x0001=read, 0x0002=write, 0x0008=create
    payload[plen++] = (unsigned char)(mode & 0xFF);
    payload[plen++] = (unsigned char)((mode >> 8) & 0xFF);

    // Permission mode (Unix-style, 0644)
    payload[plen++] = 0xA4;
    payload[plen++] = 0x01;

    // Filename (null-terminated)
    flen = (int)strlen(filename);
    if (flen > 200) flen = 200;
    memcpy(payload + plen, filename, flen);
    plen += flen;
    payload[plen++] = 0x00;

    n = fn_tnfs_transaction(ch, 0x29, payload, plen,
                            resp, sizeof(resp));
    if (n < 6) return FN_ERR_IO_ERROR;

    ch->tnfs_fd = resp[5];
    return FN_ERR_OK;
}

 // Read from a TNFS file.
 // TNFS CMD 0x21 = READ
static int fn_tnfs_read(FnChannel *ch, void *buf, int len)
{
    unsigned char payload[4];
    unsigned char resp[FN_TNFS_PACKET_MAX];
    int n;
    int data_len;

    if (len > FN_TNFS_PACKET_MAX - 6)
        len = FN_TNFS_PACKET_MAX - 6;

    payload[0] = (unsigned char)ch->tnfs_fd;
    payload[1] = (unsigned char)(len & 0xFF);
    payload[2] = (unsigned char)((len >> 8) & 0xFF);

    n = fn_tnfs_transaction(ch, 0x21, payload, 3,
                            resp, sizeof(resp));
    if (n < 5) return -1;

    data_len = n - 5;
    if (data_len > len) data_len = len;
    if (data_len > 0)
        memcpy(buf, resp + 5, data_len);

    return data_len;
}

 // Write to a TNFS file.
 // TNFS CMD 0x22 = WRITE
static int fn_tnfs_write(FnChannel *ch,
    const void *buf, int len)
{
    unsigned char payload[FN_TNFS_PACKET_MAX];
    unsigned char resp[FN_TNFS_PACKET_MAX];
    int plen = 0;
    int n;

    if (len > FN_TNFS_PACKET_MAX - 8)
        len = FN_TNFS_PACKET_MAX - 8;

    payload[plen++] = (unsigned char)ch->tnfs_fd;
    payload[plen++] = (unsigned char)(len & 0xFF);
    payload[plen++] = (unsigned char)((len >> 8) & 0xFF);
    memcpy(payload + plen, buf, len);
    plen += len;

    n = fn_tnfs_transaction(ch, 0x22, payload, plen,
                            resp, sizeof(resp));
    if (n < 5) return -1;

    // Response byte 5-6 = bytes actually written
    return (int)resp[5] | ((int)resp[6] << 8);
}

 // Close a TNFS file handle.
 // TNFS CMD 0x23 = CLOSE
static int fn_tnfs_close_file(FnChannel *ch)
{
    unsigned char payload[1];
    unsigned char resp[32];

    payload[0] = (unsigned char)ch->tnfs_fd;
    fn_tnfs_transaction(ch, 0x23, payload, 1,
                        resp, sizeof(resp));
    ch->tnfs_fd = -1;
    return FN_ERR_OK;
}

 // Unmount a TNFS session.
 // TNFS CMD 0x01 = UMOUNT
static int fn_tnfs_unmount(FnChannel *ch)
{
    unsigned char resp[32];
    fn_tnfs_transaction(ch, 0x01, NULL, 0,
                        resp, sizeof(resp));
    ch->tnfs_session[0] = 0;
    ch->tnfs_session[1] = 0;
    return FN_ERR_OK;
}

// ================================================================
 // SIMPLE JSON PARSER
 //
 // Supports JSONPath-like queries: /key1/key2/array[n]/key3
 // Returns the raw value (string, number, boolean) as text.
 // FujiNet does this parsing on the ESP32 hardware - we do
 // it in software on the desktop.
 // ================================================================ 

 // Skip whitespace in JSON text.
static const char *json_skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t' ||
           *p == '\n' || *p == '\r')
        p++;
    return p;
}

 // Skip a JSON value (string, number, object, array, bool, null).
 // Returns pointer past the end of the value.
static const char *json_skip_value(const char *p)
{
    int depth;

    p = json_skip_ws(p);

    if (*p == '"') {
        // String: skip to closing quote
        p++;
        while (*p && *p != '"') {
            if (*p == '\\') p++; // skip escape
            if (*p) p++;
        }
        if (*p == '"') p++;
        return p;
    }

    if (*p == '{' || *p == '[') {
        // Object or array: count braces
        char open = *p;
        char close_ch = (open == '{') ? '}' : ']';
        depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\') p++;
                    if (*p) p++;
                }
                if (*p == '"') p++;
                continue;
            }
            if (*p == open) depth++;
            else if (*p == close_ch) depth--;
            p++;
        }
        return p;
    }

    // Number, boolean, or null: skip to delimiter
    while (*p && *p != ',' && *p != '}' &&
           *p != ']' && *p != ' ' && *p != '\t' &&
           *p != '\n' && *p != '\r')
        p++;
    return p;
}

 // Query a JSON document with a simple path.
 // Path format: /key1/key2/[0]/key3
 //
 // Returns a pointer to the start of the matched value
 // within the json string, or NULL on failure.
static const char *fn_json_query(const char *json,
    const char *path)
{
    const char *p = json;
    const char *seg_start;
    int seg_len;
    char seg[128];

    if (!json || !path) return NULL;
    p = json_skip_ws(p);

    // Walk path segments separated by '/'
    if (*path == '/') path++;

    while (*path) {
        // Extract next path segment
        seg_start = path;
        while (*path && *path != '/') path++;
        seg_len = (int)(path - seg_start);
        if (seg_len > 127) seg_len = 127;
        memcpy(seg, seg_start, seg_len);
        seg[seg_len] = '\0';
        if (*path == '/') path++;

        p = json_skip_ws(p);

        // Check if this is an array index [n]
        if (seg[0] == '[' || (seg[0] >= '0' && seg[0] <= '9')) {
            int idx;
            int cur = 0;

            if (seg[0] == '[') {
                idx = atoi(seg + 1);
            } else {
                idx = atoi(seg);
            }

            if (*p != '[') return NULL;
            p++; // skip '['

            while (cur < idx) {
                p = json_skip_ws(p);
                if (*p == ']') return NULL;
                p = json_skip_value(p);
                p = json_skip_ws(p);
                if (*p == ',') p++;
                cur++;
            }
            p = json_skip_ws(p);
            // p now points to the idx-th element
            continue;
        }

        // Object key lookup
        if (*p != '{') return NULL;
        p++; // skip '{'

        while (1) {
            const char *key_start;
            int key_len;

            p = json_skip_ws(p);
            if (*p == '}' || *p == '\0') return NULL;

            // Read key
            if (*p != '"') return NULL;
            p++;
            key_start = p;
            while (*p && *p != '"') {
                if (*p == '\\') p++;
                if (*p) p++;
            }
            key_len = (int)(p - key_start);
            if (*p == '"') p++;

            // Skip colon
            p = json_skip_ws(p);
            if (*p == ':') p++;
            p = json_skip_ws(p);

            // Check if key matches segment
            if (key_len == seg_len &&
                memcmp(key_start, seg, seg_len) == 0) {
                // Found the key - p points to its value
                break;
            }

            // Skip the value and move to next key
            p = json_skip_value(p);
            p = json_skip_ws(p);
            if (*p == ',') p++;
        }
    }

    return p;
}

 // Extract a JSON value into a string buffer.
 // Strips outer quotes from strings.
static int fn_json_extract(const char *value_ptr,
    char *buf, int buf_max)
{
    const char *end;
    int len;

    if (!value_ptr || !buf || buf_max <= 0)
        return -1;

    value_ptr = json_skip_ws(value_ptr);

    if (*value_ptr == '"') {
        // String value - extract without quotes
        value_ptr++;
        end = value_ptr;
        while (*end && *end != '"') {
            if (*end == '\\') end++;
            if (*end) end++;
        }
        len = (int)(end - value_ptr);
        if (len >= buf_max) len = buf_max - 1;
        memcpy(buf, value_ptr, len);
        buf[len] = '\0';
        return len;
    }

    // Non-string value (number, bool, null, object, array)
    end = json_skip_value(value_ptr);
    len = (int)(end - value_ptr);
    if (len >= buf_max) len = buf_max - 1;
    memcpy(buf, value_ptr, len);
    buf[len] = '\0';
    return len;
}

// ================================================================
 // TELNET IAC NEGOTIATION
 //
 // Handles Telnet command sequences (IAC WILL/WONT/DO/DONT).
 // Responds with WONT/DONT to decline all options, which is
 // the simplest compliant Telnet client behavior.
 // ================================================================ 

static int fn_telnet_filter(FnChannel *ch,
    const unsigned char *raw, int raw_len,
    unsigned char *clean, int *clean_len)
{
    int ri = 0;
    int ci = 0;

    while (ri < raw_len) {
        if (raw[ri] == 0xFF && ri + 2 < raw_len) {
            // IAC command
            unsigned char cmd = raw[ri + 1];
            unsigned char opt = raw[ri + 2];
            unsigned char resp[3];

            resp[0] = 0xFF;
            if (cmd == 0xFB || cmd == 0xFD) {
                // WILL or DO - respond with DONT/WONT
                resp[1] = (cmd == 0xFB) ? 0xFE : 0xFC;
                resp[2] = opt;
                send(ch->sock_fd, (char *)resp, 3, 0);
            }
            ri += 3;
            continue;
        }

        if (ci < *clean_len)
            clean[ci++] = raw[ri];
        ri++;
    }

    *clean_len = ci;
    return ci;
}

// ================================================================
 // BASE64 CODEC
 //
 // FujiNet provides hardware-accelerated Base64 on the ESP32.
 // Desktop implementation is pure software.
 // ================================================================ 

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrst"
    "uvwxyz0123456789+/";

static int fn_base64_encode(const unsigned char *src, int src_len,
    char *dst, int dst_max)
{
    int i = 0;
    int o = 0;
    unsigned int triple;

    while (i < src_len && o + 4 < dst_max) {
        triple = (unsigned int)src[i++] << 16;
        if (i < src_len) triple |= (unsigned int)src[i++] << 8;
        if (i < src_len) triple |= (unsigned int)src[i++];

        dst[o++] = b64_table[(triple >> 18) & 0x3F];
        dst[o++] = b64_table[(triple >> 12) & 0x3F];
        dst[o++] = (i > src_len + 1) ? '='
                 : b64_table[(triple >> 6) & 0x3F];
        dst[o++] = (i > src_len) ? '='
                 : b64_table[triple & 0x3F];
    }
    dst[o] = '\0';
    return o;
}

static int fn_base64_decode_char(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static int fn_base64_decode(const char *src, int src_len,
    unsigned char *dst, int dst_max)
{
    int i = 0;
    int o = 0;
    unsigned int accum;
    int bits;
    int val;

    while (i < src_len && o < dst_max) {
        accum = 0;
        bits = 0;
        while (bits < 24 && i < src_len) {
            if (src[i] == '=' || src[i] == '\0') {
                i = src_len;
                break;
            }
            val = fn_base64_decode_char(src[i++]);
            if (val < 0) continue;
            accum = (accum << 6) | (unsigned int)val;
            bits += 6;
        }
        if (bits >= 8 && o < dst_max)
            dst[o++] = (unsigned char)((accum >> (bits - 8)) & 0xFF);
        bits -= 8;
        if (bits >= 8 && o < dst_max)
            dst[o++] = (unsigned char)((accum >> (bits - 8)) & 0xFF);
        bits -= 8;
        if (bits >= 8 && o < dst_max)
            dst[o++] = (unsigned char)(accum & 0xFF);
    }
    return o;
}

// ================================================================
 // HTTPS (limited) -- via system curl
 //
 // Both Windows 11 and Linux ship with curl. We use popen()
 // to invoke it, capturing the response into the channel's
 // http_body buffer. This gives real TLS without any library
 // dependency. "Limited" because it's fetch-and-buffer, not
 // streaming, and does not support IOCTL-based header control.
 // ================================================================ 

 // fn_url_is_safe -- reject shell metacharacters.
 // Returns 1 if safe, 0 if the URL contains injection risk.
static int fn_url_is_safe(const char *url)
{
    const char *p;
    for (p = url; *p; p++) {
        switch (*p) {
        case ';': case '|': case '&': case '`':
        case '$': case '(': case ')': case '{':
        case '}': case '<': case '>': case '!':
        case '\'': case '\\': case '\n': case '\r':
            return 0;
        }
    }
    return 1;
}

 // fn_https_fetch -- fetch an HTTPS URL via system curl.
 //
 // Builds a curl command, executes it via popen, and captures
 // the entire response (headers + body) into ch->http_body.
 // Parses out the HTTP status code and body offset.
static int fn_https_fetch(FnChannel *ch, const char *method)
{
#ifdef FN_NO_NETWORKING
    (void)ch; (void)method;
    return FN_ERR_NOT_IMPL;
#else
    char cmd[1024];
    FILE *pipe;
    char *buf;
    int capacity = 32768;
    int total = 0;
    int n;
    const char *hdr_end;
    const char *status_line;

    // Sanitize URL
    if (!fn_url_is_safe(ch->host) ||
        !fn_url_is_safe(ch->path)) {
        return FN_ERR_INVALID_URL;
    }

    // Build curl command:
     // -s: silent (no progress)
     // -i: include response headers
     // -L: follow redirects
     // -X: HTTP method
     // --max-time 30: timeout
     //
     // NOTE: On Windows, PowerShell aliases 'curl' to
     // Invoke-WebRequest, so we must use 'curl.exe'.
#ifdef _WIN32
#define FN_CURL "curl.exe"
#else
#define FN_CURL "curl"
#endif
    if (ch->path[0] == '/')
        snprintf(cmd, sizeof(cmd),
            FN_CURL " -s -i -L --max-time 30 -X %s "
            "\"https://%s:%d%s\" 2>&1",
            method, ch->host, ch->port, ch->path);
    else
        snprintf(cmd, sizeof(cmd),
            FN_CURL " -s -i -L --max-time 30 -X %s "
            "\"https://%s:%d/%s\" 2>&1",
            method, ch->host, ch->port, ch->path);
#undef FN_CURL

    pipe = fn_popen(cmd, "r");
    if (pipe == NULL) {
        ch->last_error = FN_ERR_IO_ERROR;
        return FN_ERR_IO_ERROR;
    }

    // Read entire response
    buf = (char *)malloc((size_t)capacity);
    if (buf == NULL) {
        fn_pclose(pipe);
        return FN_ERR_IO_ERROR;
    }

    while ((n = (int)fread(buf + total, 1,
            (size_t)(capacity - total - 1), pipe)) > 0) {
        total += n;
        if (total >= capacity - 1) {
            int newcap = capacity * 2;
            char *tmp = (char *)realloc(buf, (size_t)newcap);
            if (tmp == NULL) break;
            buf = tmp;
            capacity = newcap;
        }
    }
    buf[total] = '\0';
    fn_pclose(pipe);

    // Parse HTTP status code from first line
    ch->http_status_code = 0;
    status_line = buf;
    if (strncmp(status_line, "HTTP/", 5) == 0) {
        const char *sp = strchr(status_line, ' ');
        if (sp != NULL)
            ch->http_status_code = atoi(sp + 1);
    }

    // Find header/body separator
    hdr_end = strstr(buf, "\r\n\r\n");
    if (hdr_end != NULL) {
        int hdr_len = (int)(hdr_end - buf);
        int body_start = hdr_len + 4;

        // Copy headers
        if (hdr_len >= FN_HTTP_HEADER_MAX)
            hdr_len = FN_HTTP_HEADER_MAX - 1;
        memcpy(ch->http_headers, buf, (size_t)hdr_len);
        ch->http_headers[hdr_len] = '\0';

        // Copy body
        ch->http_body_len = total - body_start;
        if (ch->http_body != NULL) free(ch->http_body);
        ch->http_body = (char *)malloc(
            (size_t)(ch->http_body_len + 1));
        if (ch->http_body != NULL) {
            memcpy(ch->http_body, buf + body_start,
                (size_t)ch->http_body_len);
            ch->http_body[ch->http_body_len] = '\0';
        }
        ch->http_body_pos = 0;
    } else {
        // No header separator -- treat all as body
        ch->http_body_len = total;
        if (ch->http_body != NULL) free(ch->http_body);
        ch->http_body = buf;
        buf = NULL; // ownership transferred
        ch->http_body_pos = 0;
    }

    if (buf != NULL) free(buf);
    ch->connected = 1;
    ch->eof_flag = 0;
    return FN_ERR_OK;
#endif
}

// ================================================================
 // SSH (limited) -- TCP connect + version exchange
 //
 // Per RFC 4253, the SSH connection begins with both sides
 // sending a version string:
 //   SSH-protoversion-softwareversion SP comments CR LF
 //
 // We connect, read the server banner, send our version,
 // then switch the channel to telnet-like raw interactive
 // mode so the user can at least see server output. Actual
 // SSH key exchange and encryption require a crypto library.
 //
 // After version exchange, the channel operates in raw TCP
 // mode -- same as FN_PROTO_TCP. This allows the SSH "limited"
 // to at minimum identify the server and provide raw I/O.
 // ================================================================ 

static int fn_ssh_connect(FnChannel *ch)
{
#ifdef FN_NO_NETWORKING
    (void)ch;
    return FN_ERR_NOT_IMPL;
#else
    int fd;
    int n;
    char banner[256];
    const char *our_version = "SSH-2.0-BASIC++_1.0\r\n";

    fd = fn_tcp_connect(ch->host, ch->port);
    if (fd < 0) return FN_ERR_CONN_REFUSED;

    ch->sock_fd = fd;
    ch->connected = 1;

    // Read server version string (up to 255 bytes per RFC 4253)
    memset(banner, 0, sizeof(banner));
    n = recv(fd, banner, sizeof(banner) - 1, 0);
    if (n <= 0) {
        fn_closesocket(fd);
        ch->sock_fd = -1;
        ch->connected = 0;
        return FN_ERR_CONN_REFUSED;
    }
    banner[n] = '\0';

    // Strip trailing CR/LF
    while (n > 0 && (banner[n-1] == '\r' || banner[n-1] == '\n'))
        banner[--n] = '\0';

    // Store server SSH version
    strncpy(ch->ssh_version, banner, sizeof(ch->ssh_version) - 1);
    ch->ssh_version[sizeof(ch->ssh_version) - 1] = '\0';
    ch->ssh_exchanged = 1;

    // Send our version string
    send(fd, our_version, (int)strlen(our_version), 0);

    // Put the server banner into recv_buf so the user can
     // read it with INPUT #ch, V$.
     // After that, the channel operates in raw TCP mode
     // (telnet-like) for any further data the server sends. 
    {
        int blen = (int)strlen(ch->ssh_version);
        if (blen >= FN_RECV_BUF_SIZE)
            blen = FN_RECV_BUF_SIZE - 1;
        memcpy(ch->recv_buf, ch->ssh_version, (size_t)blen);
        ch->recv_buf[blen] = '\n';
        ch->recv_pos = 0;
        ch->recv_len = blen + 1;
    }

    return FN_ERR_OK;
#endif
}

// ================================================================
 // N: DEVICE -- VIRTUAL DEVICE CALLBACKS
 // ================================================================ 

 // N: open -- Parse devicespec and connect.
static int fn_net_open(VDev *d, const char *path,
    const char *mode)
{
    FnChannel *ch;
    int slot;
    int fd;
    FnProto proto;
    char host[128];
    int port = 0;
    char url_path[FN_MAX_URL];
    int rc;

    (void)d;

    slot = fn_alloc_channel();
    if (slot < 0) return FN_ERR_CHANNEL_FULL;

    ch = &fn_channels[slot];
    memset(ch, 0, sizeof(FnChannel));
    ch->sock_fd = -1;
    ch->tnfs_fd = -1;

    // Store devicespec
    strncpy(ch->devicespec, path, FN_MAX_DEVICESPEC - 1);

    // Parse the URL
    rc = fn_parse_devicespec(path, &proto, host, sizeof(host),
                             &port, url_path, sizeof(url_path));
    if (rc != 0) return FN_ERR_INVALID_URL;

    ch->proto = proto;
    strncpy(ch->host, host, sizeof(ch->host) - 1);
    ch->port = port;
    strncpy(ch->path, url_path, sizeof(ch->path) - 1);

    // Parse mode string
    ch->mode = FN_MODE_READWRITE;
    if (mode) {
        if (mode[0] == 'R' || mode[0] == 'r') {
            ch->mode = FN_MODE_READ;
            if (mode[1] == 'W' || mode[1] == 'w')
                ch->mode = FN_MODE_READWRITE;
        } else if (mode[0] == 'W' || mode[0] == 'w') {
            ch->mode = FN_MODE_WRITE;
        } else if (mode[0] == 'A' || mode[0] == 'a') {
            ch->mode = FN_MODE_APPEND;
        }
    }

    ch->translation = FN_TRANS_NONE;

    // Connect based on protocol
    switch (proto) {
    case FN_PROTO_TCP:
    case FN_PROTO_TELNET:
        fd = fn_tcp_connect(host, port);
        if (fd < 0) return FN_ERR_CONN_REFUSED;
        ch->sock_fd = fd;
        ch->connected = 1;
        break;

    case FN_PROTO_UDP:
    case FN_PROTO_TNFS:
        fd = fn_udp_create(host, port);
        if (fd < 0) return FN_ERR_CONN_REFUSED;
        ch->sock_fd = fd;
        ch->connected = 1;
        // TNFS requires a mount step
        if (proto == FN_PROTO_TNFS) {
            rc = fn_tnfs_mount(ch);
            if (rc != FN_ERR_OK) {
                fn_closesocket(fd);
                ch->sock_fd = -1;
                return rc;
            }
        }
        break;

    case FN_PROTO_HTTP:
        // HTTP: connect, send request, buffer response
        fd = fn_tcp_connect(host, port);
        if (fd < 0) return FN_ERR_CONN_REFUSED;
        ch->sock_fd = fd;
        ch->connected = 1;
        rc = fn_http_request(ch, "GET");
        if (rc != FN_ERR_OK) {
            fn_closesocket(fd);
            ch->sock_fd = -1;
            return rc;
        }
        break;

    case FN_PROTO_HTTPS:
        // HTTPS (limited): use system curl for TLS
        ch->sock_fd = -1; // no raw socket -- curl handles it
        rc = fn_https_fetch(ch, "GET");
        if (rc != FN_ERR_OK) return rc;
        break;

    case FN_PROTO_SSH:
        // SSH (limited): version exchange + raw TCP mode
        rc = fn_ssh_connect(ch);
        if (rc != FN_ERR_OK) return rc;
        break;

    case FN_PROTO_FTP:
        // FTP: raw TCP command channel
        fd = fn_tcp_connect(host, port);
        if (fd < 0) return FN_ERR_CONN_REFUSED;
        ch->sock_fd = fd;
        ch->connected = 1;
        break;

    default:
        return FN_ERR_NOT_IMPL;
    }

    ch->in_use = 1;
    ch->eof_flag = 0;
    ch->last_error = FN_ERR_OK;

    // Return channel number as the "file descriptor"
     // via user_data so the caller can retrieve it 
    d->user_data = (void *)(intptr_t)slot;

    return FN_ERR_OK;
}

 // N: close -- Disconnect and free the channel.
static int fn_net_close(VDev *d)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;

    if (slot < 0 || slot >= FN_MAX_CHANNELS)
        return FN_ERR_NOT_OPEN;

    ch = &fn_channels[slot];
    if (!ch->in_use)
        return FN_ERR_NOT_OPEN;

    // Protocol-specific teardown
    if (ch->proto == FN_PROTO_TNFS) {
        if (ch->tnfs_fd >= 0)
            fn_tnfs_close_file(ch);
        fn_tnfs_unmount(ch);
    }

    if (ch->sock_fd >= 0) {
        fn_closesocket(ch->sock_fd);
        ch->sock_fd = -1;
    }

    if (ch->http_body) {
        free(ch->http_body);
        ch->http_body = NULL;
    }
    if (ch->json_data) {
        free(ch->json_data);
        ch->json_data = NULL;
    }

    ch->in_use = 0;
    ch->connected = 0;
    d->user_data = (void *)(intptr_t)(-1);

    return FN_ERR_OK;
}

 // N: read -- Read binary data from the channel.
static int fn_net_read(VDev *d, void *buf, int len)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;
    int n;

    if (slot < 0 || slot >= FN_MAX_CHANNELS)
        return -1;
    ch = &fn_channels[slot];
    if (!ch->in_use || !ch->connected) return -1;

    // HTTP: read from buffered body
    if (ch->proto == FN_PROTO_HTTP ||
        ch->proto == FN_PROTO_HTTPS) {
        int avail = ch->http_body_len - ch->http_body_pos;
        if (avail <= 0) { ch->eof_flag = 1; return 0; }
        if (len > avail) len = avail;
        memcpy(buf, ch->http_body + ch->http_body_pos, len);
        ch->http_body_pos += len;
        ch->bytes_waiting = (unsigned int)(
            ch->http_body_len - ch->http_body_pos);
        return len;
    }

    // TNFS: use TNFS read command
    if (ch->proto == FN_PROTO_TNFS) {
        n = fn_tnfs_read(ch, buf, len);
        if (n <= 0) ch->eof_flag = 1;
        return n;
    }

    // TCP/UDP/Telnet: read from socket
    n = recv(ch->sock_fd, (char *)buf, len, 0);

    if (n <= 0) {
        ch->eof_flag = 1;
        ch->connected = 0;
        return 0;
    }

    // Telnet IAC filtering
    if (ch->proto == FN_PROTO_TELNET) {
        unsigned char filtered[FN_RECV_BUF_SIZE];
        int filtered_len = sizeof(filtered);
        fn_telnet_filter(ch, (unsigned char *)buf, n,
                         filtered, &filtered_len);
        if (filtered_len > len) filtered_len = len;
        memcpy(buf, filtered, filtered_len);
        n = filtered_len;
    }

    ch->bytes_waiting = 0;
    return n;
}

 // N: write -- Write binary data to the channel.
static int fn_net_write(VDev *d, const void *buf, int len)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;
    int n;

    if (slot < 0 || slot >= FN_MAX_CHANNELS)
        return -1;
    ch = &fn_channels[slot];
    if (!ch->in_use || !ch->connected) return -1;

    // TNFS: use TNFS write command
    if (ch->proto == FN_PROTO_TNFS) {
        return fn_tnfs_write(ch, buf, len);
    }

    // HTTP POST body: buffer it for later sending
    if (ch->proto == FN_PROTO_HTTP ||
        ch->proto == FN_PROTO_HTTPS) {
        if (ch->http_body) free(ch->http_body);
        ch->http_body = (char *)malloc(len + 1);
        if (!ch->http_body) return -1;
        memcpy(ch->http_body, buf, len);
        ch->http_body[len] = '\0';
        ch->http_body_len = len;
        return len;
    }

    // TCP/UDP/Telnet: write to socket
    n = send(ch->sock_fd, (const char *)buf, len, 0);
    if (n <= 0) {
        ch->connected = 0;
        return -1;
    }
    return n;
}

 // N: putc -- Write a single character (line-mode output).
static int fn_net_putc(VDev *d, int ch_char)
{
    char c = (char)ch_char;
    return fn_net_write(d, &c, 1);
}

 // N: puts -- Write a string.
static int fn_net_puts(VDev *d, const char *s)
{
    if (!s) return 0;
    return fn_net_write(d, s, (int)strlen(s));
}

 // N: gets -- Read a line (up to newline or max).
static int fn_net_gets(VDev *d, char *buf, int max)
{
    int i = 0;
    int n;
    char c;

    while (i < max - 1) {
        n = fn_net_read(d, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

 // N: getc -- Read a single character.
static int fn_net_getc(VDev *d)
{
    char c;
    int n = fn_net_read(d, &c, 1);
    if (n <= 0) return -1;
    return (int)(unsigned char)c;
}

 // N: status -- Check connection status.
static int fn_net_status(VDev *d)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;

    if (slot < 0 || slot >= FN_MAX_CHANNELS) return -1;
    ch = &fn_channels[slot];
    if (!ch->in_use) return -1;

    return ch->connected ? 0 : -1;
}

 // N: poll -- Check for data availability (non-blocking).
static int fn_net_poll(VDev *d)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;

    if (slot < 0 || slot >= FN_MAX_CHANNELS) return -1;
    ch = &fn_channels[slot];

    // HTTP: check buffered body
    if (ch->proto == FN_PROTO_HTTP ||
        ch->proto == FN_PROTO_HTTPS) {
        return (ch->http_body_pos < ch->http_body_len) ? 1 : 0;
    }

    // Socket: use non-blocking recv peek
#ifndef FN_NO_NETWORKING
    if (ch->sock_fd >= 0) {
        char tmp;
        int n;
        fn_set_nonblocking(ch->sock_fd);
        n = recv(ch->sock_fd, &tmp, 1, MSG_PEEK);
        return (n > 0) ? 1 : 0;
    }
#endif

    return 0;
}

 // N: ioctl -- Extended network control commands.
 //
 // This is where FujiNet-specific operations live:
 // JSON parsing, HTTP methods, translation modes, etc.
static int fn_net_ioctl(VDev *d, int cmd, void *arg)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;
    int rc;

    if (slot < 0 || slot >= FN_MAX_CHANNELS) return -1;
    ch = &fn_channels[slot];

    switch (cmd) {

    case FNIO_JSON_PARSE:
         // Read all available data and parse as JSON.
         // On real FujiNet, the ESP32 does this parsing.
        if (ch->json_data) { free(ch->json_data); ch->json_data = NULL; }

        if (ch->proto == FN_PROTO_HTTP ||
            ch->proto == FN_PROTO_HTTPS) {
            // Use HTTP response body
            if (ch->http_body && ch->http_body_len > 0) {
                ch->json_data = (char *)malloc(
                    ch->http_body_len + 1);
                if (ch->json_data) {
                    memcpy(ch->json_data, ch->http_body,
                           ch->http_body_len);
                    ch->json_data[ch->http_body_len] = '\0';
                    ch->json_data_len = ch->http_body_len;
                    ch->json_parsed = 1;
                    return FN_ERR_OK;
                }
            }
        } else {
            // TCP/UDP: read until EOF into buffer
            char *buf = (char *)malloc(FN_HTTP_BODY_MAX);
            int total = 0;
            int n;
            if (!buf) return FN_ERR_BUF_OVERFLOW;
            while (total < FN_HTTP_BODY_MAX - 1) {
                n = recv(ch->sock_fd, buf + total,
                         FN_HTTP_BODY_MAX - 1 - total, 0);
                if (n <= 0) break;
                total += n;
            }
            buf[total] = '\0';
            ch->json_data = buf;
            ch->json_data_len = total;
            ch->json_parsed = 1;
            return FN_ERR_OK;
        }
        return FN_ERR_JSON_PARSE;

    case FNIO_JSON_QUERY:
         // Query parsed JSON with a path expression.
         // arg = pointer to struct { char *query; char *result; int max; }
        if (!ch->json_parsed || !ch->json_data)
            return FN_ERR_JSON_PARSE;
        if (arg) {
            // arg points to: query_str, result_buf, max_len
            char **args = (char **)arg;
            const char *val;
            val = fn_json_query(ch->json_data, args[0]);
            if (val) {
                fn_json_extract(val, args[1],
                    *(int *)(args + 2));
                return FN_ERR_OK;
            }
        }
        return FN_ERR_JSON_PARSE;

    case FNIO_SET_CHANNEL_MODE:
        if (arg) ch->mode = *(int *)arg;
        return FN_ERR_OK;

    case FNIO_SET_TRANSLATION:
        if (arg) ch->translation = *(int *)arg;
        return FN_ERR_OK;

    case FNIO_GET_BYTES_WAITING:
        if (arg) *(unsigned int *)arg = ch->bytes_waiting;
        return FN_ERR_OK;

    case FNIO_GET_CONNECTED:
        if (arg) *(int *)arg = ch->connected;
        return FN_ERR_OK;

    case FNIO_GET_ERROR:
        if (arg) *(int *)arg = ch->last_error;
        return FN_ERR_OK;

    case FNIO_HTTP_SET_HEADER:
        if (arg) {
            strncpy(ch->http_headers, (const char *)arg,
                    FN_HTTP_HEADER_MAX - 1);
        }
        return FN_ERR_OK;

    case FNIO_HTTP_POST:
        if (ch->sock_fd < 0) return FN_ERR_NOT_OPEN;
        rc = fn_http_request(ch, "POST");
        return rc;

    case FNIO_HTTP_PUT:
        if (ch->sock_fd < 0) return FN_ERR_NOT_OPEN;
        rc = fn_http_request(ch, "PUT");
        return rc;

    case FNIO_HTTP_DELETE:
        if (ch->sock_fd < 0) return FN_ERR_NOT_OPEN;
        rc = fn_http_request(ch, "DELETE");
        return rc;

    case FNIO_BASE64_ENCODE:
        if (arg) {
            char **args = (char **)arg;
            fn_base64_encode((unsigned char *)args[0],
                (int)strlen(args[0]),
                args[1], *(int *)(args + 2));
            return FN_ERR_OK;
        }
        return FN_ERR_BAD_CMD;

    case FNIO_BASE64_DECODE:
        if (arg) {
            char **args = (char **)arg;
            fn_base64_decode(args[0], (int)strlen(args[0]),
                (unsigned char *)args[1],
                *(int *)(args + 2));
            return FN_ERR_OK;
        }
        return FN_ERR_BAD_CMD;

    default:
        return FN_ERR_BAD_CMD;
    }
}

 // N: info -- Return device metadata.
static const char *fn_net_info(VDev *d, const char *key)
{
    int slot = (int)(intptr_t)d->user_data;
    FnChannel *ch;
    static char info_buf[64];

    if (slot < 0 || slot >= FN_MAX_CHANNELS) return NULL;
    ch = &fn_channels[slot];

    if (strcmp(key, "proto") == 0) {
        switch (ch->proto) {
        case FN_PROTO_TCP:    return "TCP";
        case FN_PROTO_UDP:    return "UDP";
        case FN_PROTO_HTTP:   return "HTTP";
        case FN_PROTO_HTTPS:  return "HTTPS";
        case FN_PROTO_TNFS:   return "TNFS";
        case FN_PROTO_FTP:    return "FTP";
        case FN_PROTO_TELNET: return "TELNET";
        case FN_PROTO_SSH:    return "SSH";
        default: return "UNKNOWN";
        }
    }
    if (strcmp(key, "host") == 0) return ch->host;
    if (strcmp(key, "path") == 0) return ch->path;
    if (strcmp(key, "port") == 0) {
        sprintf(info_buf, "%d", ch->port);
        return info_buf;
    }
    if (strcmp(key, "connected") == 0)
        return ch->connected ? "1" : "0";
    if (strcmp(key, "eof") == 0)
        return ch->eof_flag ? "1" : "0";
    if (strcmp(key, "http_status") == 0) {
        sprintf(info_buf, "%d", ch->http_status_code);
        return info_buf;
    }
    if (strcmp(key, "ssh_version") == 0)
        return ch->ssh_version[0] ? ch->ssh_version : NULL;
    if (strcmp(key, "bytes_waiting") == 0) {
        sprintf(info_buf, "%d", ch->recv_len - ch->recv_pos);
        return info_buf;
    }
    if (strcmp(key, "error") == 0) {
        sprintf(info_buf, "%d", ch->last_error);
        return info_buf;
    }

    return NULL;
}

// ================================================================
 // FUJI: DEVICE -- VIRTUAL DEVICE CALLBACKS
 //
 // Configuration device for WiFi, host/device slots, AppKey
 // storage, directory browsing, and adapter status.
 //
 // On desktop, WiFi operations report "connected" using the
 // host OS network stack. Host slots and AppKey data are stored
 // in a local config file (~/.basicpp/fujinet.cfg).
 // ================================================================ 

 // FUJI: Initialize adapter configuration from platform APIs.
static void fn_fuji_init_config(void)
{
    memset(&fn_fuji, 0, sizeof(fn_fuji));
    fn_fuji.wifi_status = FN_WIFI_CONNECTED;
    strcpy(fn_fuji.ssid, "(desktop)");
    strcpy(fn_fuji.hostname, "basicpp");
    strcpy(fn_fuji.fn_version, "1.0-desktop");

    // Local IP = 127.0.0.1
    fn_fuji.local_ip[0] = 127;
    fn_fuji.local_ip[3] = 1;
    fn_fuji.gateway[0] = 192;
    fn_fuji.gateway[1] = 168;
    fn_fuji.gateway[2] = 1;
    fn_fuji.gateway[3] = 1;
    fn_fuji.netmask[0] = 255;
    fn_fuji.netmask[1] = 255;
    fn_fuji.netmask[2] = 255;
    fn_fuji.dns[0] = 8;
    fn_fuji.dns[1] = 8;
    fn_fuji.dns[2] = 8;
    fn_fuji.dns[3] = 8;

    fn_fuji.initialized = 1;
}

 // FUJI: Load host slots from local config file.
static void fn_fuji_load_slots(void)
{
    FILE *fp;
    int i;
    char path[256];

#ifdef _WIN32
    {
        const char *home = getenv("USERPROFILE");
        if (!home) home = ".";
        sprintf(path, "%s\\.basicpp\\fujinet_slots.cfg", home);
    }
#else
    {
        const char *home = getenv("HOME");
        if (!home) home = ".";
        sprintf(path, "%s/.basicpp/fujinet_slots.cfg", home);
    }
#endif

    fp = fopen(path, "r");
    if (fp) {
        for (i = 0; i < FN_MAX_HOST_SLOTS; i++) {
            if (fgets(fn_fuji.host_slots[i],
                      FN_HOST_SLOT_LEN, fp) == NULL)
                break;
            // Strip trailing newline
            {
                int len = (int)strlen(fn_fuji.host_slots[i]);
                while (len > 0 && (
                    fn_fuji.host_slots[i][len-1] == '\n' ||
                    fn_fuji.host_slots[i][len-1] == '\r'))
                    fn_fuji.host_slots[i][--len] = '\0';
            }
        }
        fclose(fp);
    }
}

 // FUJI: Save host slots to local config file.
static void fn_fuji_save_slots(void)
{
    FILE *fp;
    int i;
    char path[256];
    char dir[256];

#ifdef _WIN32
    {
        const char *home = getenv("USERPROFILE");
        if (!home) home = ".";
        sprintf(dir, "%s\\.basicpp", home);
        sprintf(path, "%s\\fujinet_slots.cfg", dir);
        // Ensure directory exists
        CreateDirectoryA(dir, NULL);
    }
#else
    {
        const char *home = getenv("HOME");
        if (!home) home = ".";
        sprintf(dir, "%s/.basicpp", home);
        sprintf(path, "%s/fujinet_slots.cfg", dir);
        mkdir(dir, 0755);
    }
#endif

    fp = fopen(path, "w");
    if (fp) {
        for (i = 0; i < FN_MAX_HOST_SLOTS; i++)
            fprintf(fp, "%s\n", fn_fuji.host_slots[i]);
        fclose(fp);
    }
}

 // FUJI: AppKey file I/O.
 // AppKeys are stored as individual files in ~/.basicpp/appkeys/
static void fn_appkey_path(unsigned int creator,
    unsigned int app, char *path, int max)
{
    char dir[256];

#ifdef _WIN32
    {
        const char *home = getenv("USERPROFILE");
        if (!home) home = ".";
        sprintf(dir, "%s\\.basicpp\\appkeys", home);
        CreateDirectoryA(dir, NULL);
        sprintf(path, "%s\\%04X_%04X.dat", dir, creator, app);
    }
#else
    {
        const char *home = getenv("HOME");
        if (!home) home = ".";
        sprintf(dir, "%s/.basicpp/appkeys", home);
        mkdir(dir, 0755);
        sprintf(path, "%s/%04X_%04X.dat", dir, creator, app);
    }
#endif
    (void)max;
}

 // FUJI: ioctl -- Handle all fuji device commands.
static int fn_fuji_ioctl(VDev *d, int cmd, void *arg)
{
    (void)d;

    switch (cmd) {

    case FNIO_RESET:
        fn_fuji_init_config();
        return FN_ERR_OK;

    case FNIO_GET_WIFI_STATUS:
        if (arg) *(int *)arg = fn_fuji.wifi_status;
        return FN_ERR_OK;

    case FNIO_GET_WIFI_ENABLED:
        if (arg) *(int *)arg = 1; // always enabled on desktop
        return FN_ERR_OK;

    case FNIO_GET_SSID:
        if (arg)
            strncpy((char *)arg, fn_fuji.ssid, FN_SSID_MAXLEN);
        return FN_ERR_OK;

    case FNIO_SET_SSID:
        if (arg) {
            // arg points to {ssid, password}
            const char *ssid = (const char *)arg;
            strncpy(fn_fuji.ssid, ssid, FN_SSID_MAXLEN - 1);
        }
        return FN_ERR_OK;

    case FNIO_SCAN_NETWORKS:
        // Desktop: scanning not supported, return 0 networks
        if (arg) *(int *)arg = 0;
        return FN_ERR_OK;

    case FNIO_GET_SCAN_RESULT:
        // Desktop: no scan results available
        return FN_ERR_NOT_IMPL;

    case FNIO_READ_HOST_SLOTS:
        if (arg)
            memcpy(arg, fn_fuji.host_slots,
                   sizeof(fn_fuji.host_slots));
        return FN_ERR_OK;

    case FNIO_WRITE_HOST_SLOTS:
        if (arg) {
            memcpy(fn_fuji.host_slots, arg,
                   sizeof(fn_fuji.host_slots));
            fn_fuji_save_slots();
        }
        return FN_ERR_OK;

    case FNIO_MOUNT_HOST:
        // Desktop: host mount is implicit (just DNS resolve)
        return FN_ERR_OK;

    case FNIO_UNMOUNT_HOST:
        return FN_ERR_OK;

    case FNIO_MOUNT_IMAGE:
        // Desktop: disk image mount not supported
        return FN_ERR_NOT_IMPL;

    case FNIO_UNMOUNT_IMAGE:
        return FN_ERR_OK;

    case FNIO_GET_ADAPTER_CONFIG:
        // Return adapter configuration struct
        if (arg) {
            // Fill a flat buffer with adapter info:
             // IP(4), gateway(4), netmask(4), dns(4),
             // mac(6), ssid(33), hostname(64), version(16) 
            unsigned char *out = (unsigned char *)arg;
            memcpy(out, fn_fuji.local_ip, 4);
            memcpy(out + 4, fn_fuji.gateway, 4);
            memcpy(out + 8, fn_fuji.netmask, 4);
            memcpy(out + 12, fn_fuji.dns, 4);
            memcpy(out + 16, fn_fuji.mac, 6);
            memcpy(out + 22, fn_fuji.ssid, FN_SSID_MAXLEN);
            memcpy(out + 55, fn_fuji.hostname, 64);
            memcpy(out + 119, fn_fuji.fn_version, 16);
        }
        return FN_ERR_OK;

    case FNIO_OPEN_APPKEY:
        if (arg) {
            unsigned int *params = (unsigned int *)arg;
            fn_fuji.appkey_creator = params[0];
            fn_fuji.appkey_app = params[1];
            fn_fuji.appkey_mode = (unsigned char)params[2];
        }
        return FN_ERR_OK;

    case FNIO_CLOSE_APPKEY:
        return FN_ERR_OK;

    case FNIO_READ_APPKEY:
        if (arg) {
            char fpath[256];
            FILE *fp;
            fn_appkey_path(fn_fuji.appkey_creator,
                           fn_fuji.appkey_app,
                           fpath, sizeof(fpath));
            fp = fopen(fpath, "rb");
            if (fp) {
                int n = (int)fread(arg, 1, FN_APPKEY_MAXLEN, fp);
                fclose(fp);
                (void)n;
                return FN_ERR_OK;
            }
            return FN_ERR_IO_ERROR;
        }
        return FN_ERR_BAD_CMD;

    case FNIO_WRITE_APPKEY:
        if (arg) {
            char fpath[256];
            FILE *fp;
            fn_appkey_path(fn_fuji.appkey_creator,
                           fn_fuji.appkey_app,
                           fpath, sizeof(fpath));
            fp = fopen(fpath, "wb");
            if (fp) {
                fwrite(arg, 1, FN_APPKEY_MAXLEN, fp);
                fclose(fp);
                return FN_ERR_OK;
            }
            return FN_ERR_IO_ERROR;
        }
        return FN_ERR_BAD_CMD;

    case FNIO_HASH_COMPUTE:
        // Desktop: hash not implemented (would need SHA lib)
        return FN_ERR_NOT_IMPL;

    case FNIO_RANDOM_NUMBER:
        if (arg) {
            *(unsigned int *)arg = (unsigned int)rand();
        }
        return FN_ERR_OK;

    case FNIO_GENERATE_GUID:
        // Generate a pseudo-GUID: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
        if (arg) {
            char *guid = (char *)arg;
            int i;
            static const char hex[] = "0123456789abcdef";
            for (i = 0; i < 36; i++) {
                if (i == 8 || i == 13 || i == 18 || i == 23)
                    guid[i] = '-';
                else if (i == 14)
                    guid[i] = '4';
                else
                    guid[i] = hex[rand() & 0x0F];
            }
            guid[36] = '\0';
        }
        return FN_ERR_OK;

    case FNIO_SET_HOST_PREFIX:
        // Store a path prefix for a host slot.
         // arg format: "slot:prefix" (e.g. "0:/games/") 
        if (arg) {
            const char *s = (const char *)arg;
            int slot_n = atoi(s);
            const char *colon = strchr(s, ':');
            if (colon && slot_n >= 0 &&
                slot_n < FN_MAX_HOST_SLOTS) {
                strncpy(fn_fuji.host_prefix[slot_n],
                    colon + 1, FN_HOST_SLOT_LEN - 1);
                fn_fuji.host_prefix[slot_n][
                    FN_HOST_SLOT_LEN - 1] = '\0';
            }
        }
        return FN_ERR_OK;

    case FNIO_GET_HOST_PREFIX:
        // Return prefix for a host slot.
         // arg is int* for slot, result written there as string 
        if (arg) {
            int slot_n = *(int *)arg;
            if (slot_n >= 0 && slot_n < FN_MAX_HOST_SLOTS)
                strcpy((char *)arg,
                    fn_fuji.host_prefix[slot_n]);
            else
                ((char *)arg)[0] = '\0';
        }
        return FN_ERR_OK;

    case FNIO_SET_DEVICE_PATH:
        // Store a path for a device slot.
         // arg format: "slot:path" 
        if (arg) {
            const char *s = (const char *)arg;
            int slot_n = atoi(s);
            const char *colon = strchr(s, ':');
            if (colon && slot_n >= 0 &&
                slot_n < FN_MAX_DEVICE_SLOTS) {
                strncpy(fn_fuji.device_path[slot_n],
                    colon + 1, FN_FILE_MAXLEN - 1);
                fn_fuji.device_path[slot_n][
                    FN_FILE_MAXLEN - 1] = '\0';
            }
        }
        return FN_ERR_OK;

    case FNIO_GET_DEVICE_PATH:
        if (arg) {
            int slot_n = *(int *)arg;
            if (slot_n >= 0 && slot_n < FN_MAX_DEVICE_SLOTS)
                strcpy((char *)arg,
                    fn_fuji.device_path[slot_n]);
            else
                ((char *)arg)[0] = '\0';
        }
        return FN_ERR_OK;

    case FNIO_READ_DEVICE_SLOTS:
        if (arg)
            memcpy(arg, fn_fuji.device_slots,
                sizeof(fn_fuji.device_slots));
        return FN_ERR_OK;

    case FNIO_WRITE_DEVICE_SLOTS:
        if (arg)
            memcpy(fn_fuji.device_slots, arg,
                sizeof(fn_fuji.device_slots));
        return FN_ERR_OK;

    case FNIO_DEVICE_ENABLE:
        if (arg) {
            int slot_n = *(int *)arg;
            if (slot_n >= 0 && slot_n < FN_MAX_DEVICE_SLOTS)
                fn_fuji.device_enabled[slot_n] = 1;
        }
        return FN_ERR_OK;

    case FNIO_DEVICE_DISABLE:
        if (arg) {
            int slot_n = *(int *)arg;
            if (slot_n >= 0 && slot_n < FN_MAX_DEVICE_SLOTS)
                fn_fuji.device_enabled[slot_n] = 0;
        }
        return FN_ERR_OK;

    case FNIO_DEVICE_STATUS:
        if (arg) {
            int slot_n = *(int *)arg;
            if (slot_n >= 0 && slot_n < FN_MAX_DEVICE_SLOTS)
                *(int *)arg = fn_fuji.device_enabled[slot_n];
            else
                *(int *)arg = 0;
        }
        return FN_ERR_OK;

    case FNIO_SET_BOOT_MODE:
        if (arg) fn_fuji.boot_mode = *(int *)arg;
        return FN_ERR_OK;

    case FNIO_COPY_FILE:
        // Copy file. arg = "src\0dst" (two null-terminated strings)
        if (arg) {
            const char *src = (const char *)arg;
            const char *dst = src + strlen(src) + 1;
            FILE *fin = fopen(src, "rb");
            FILE *fout;
            if (!fin) return FN_ERR_IO_ERROR;
            fout = fopen(dst, "wb");
            if (!fout) { fclose(fin); return FN_ERR_IO_ERROR; }
            {
                char cpbuf[4096];
                size_t nr;
                while ((nr = fread(cpbuf, 1,
                        sizeof(cpbuf), fin)) > 0) {
                    fwrite(cpbuf, 1, nr, fout);
                }
            }
            fclose(fin);
            fclose(fout);
        }
        return FN_ERR_OK;

    case FNIO_NEW_DISK:
        // Create empty file at specified path.
         // arg = "path\0size" (size in bytes as string) 
        if (arg) {
            const char *path_str = (const char *)arg;
            FILE *fp = fopen(path_str, "wb");
            if (fp) {
                const char *size_str;
                long fsize;
                size_str = path_str + strlen(path_str) + 1;
                fsize = atol(size_str);
                if (fsize > 0 && fsize <= 16777216) {
                    // Write zeros up to size
                    char zeros[512];
                    long remaining = fsize;
                    memset(zeros, 0, sizeof(zeros));
                    while (remaining > 0) {
                        size_t chunk = (size_t)(
                            remaining > 512 ? 512 :
                            remaining);
                        fwrite(zeros, 1, chunk, fp);
                        remaining -= (long)chunk;
                    }
                }
                fclose(fp);
            } else {
                return FN_ERR_IO_ERROR;
            }
        }
        return FN_ERR_OK;

    case FNIO_OPEN_DIRECTORY:
        // Open directory for browsing.
         // arg = directory path string. 
        if (fn_fuji.dir_open && fn_fuji.dir_handle) {
            // Close previously open directory
#ifdef _WIN32
            FindClose((HANDLE)fn_fuji.dir_handle);
#elif !defined(__MSDOS__) && !defined(__DOS__)
            closedir((DIR *)fn_fuji.dir_handle);
#endif
            fn_fuji.dir_handle = NULL;
            fn_fuji.dir_open = 0;
        }
        if (arg) {
            const char *dir_path = (const char *)arg;
            strncpy(fn_fuji.dir_path, dir_path,
                sizeof(fn_fuji.dir_path) - 1);
            fn_fuji.dir_path[
                sizeof(fn_fuji.dir_path) - 1] = '\0';
            fn_fuji.dir_position = 0;
#ifdef _WIN32
            {
                char pattern[272];
                WIN32_FIND_DATAA fdata;
                HANDLE h;
                snprintf(pattern, sizeof(pattern),
                    "%s\\*", dir_path);
                h = FindFirstFileA(pattern, &fdata);
                if (h == INVALID_HANDLE_VALUE)
                    return FN_ERR_IO_ERROR;
                fn_fuji.dir_handle = (void *)h;
                fn_fuji.dir_open = 1;
                // Store first entry name so first
                 // READ_DIR_ENTRY can return it 
            }
#elif !defined(__MSDOS__) && !defined(__DOS__)
            {
                DIR *dp = opendir(dir_path);
                if (!dp) return FN_ERR_IO_ERROR;
                fn_fuji.dir_handle = (void *)dp;
                fn_fuji.dir_open = 1;
            }
#else
            return FN_ERR_NOT_IMPL;
#endif
        }
        return FN_ERR_OK;

    case FNIO_READ_DIR_ENTRY:
        // Read next directory entry into arg (string buffer).
        if (!fn_fuji.dir_open || !fn_fuji.dir_handle) {
            if (arg) ((char *)arg)[0] = '\0';
            return FN_ERR_IO_ERROR;
        }
#ifdef _WIN32
        {
            WIN32_FIND_DATAA fdata;
            if (fn_fuji.dir_position == 0) {
                // First entry was already found by
                 // FindFirstFile -- re-find it 
                HANDLE h2;
                char pattern[272];
                FindClose((HANDLE)fn_fuji.dir_handle);
                snprintf(pattern, sizeof(pattern),
                    "%s\\*", fn_fuji.dir_path);
                h2 = FindFirstFileA(pattern, &fdata);
                if (h2 == INVALID_HANDLE_VALUE) {
                    fn_fuji.dir_open = 0;
                    if (arg) ((char *)arg)[0] = '\0';
                    return FN_ERR_IO_ERROR;
                }
                fn_fuji.dir_handle = (void *)h2;
                // Skip entries to reach position
                {
                    int skip = fn_fuji.dir_position;
                    while (skip > 0) {
                        if (!FindNextFileA(h2, &fdata)) {
                            if (arg) ((char *)arg)[0] = '\0';
                            return FN_ERR_EOF;
                        }
                        skip--;
                    }
                }
                if (arg) strcpy((char *)arg, fdata.cFileName);
                fn_fuji.dir_position++;
            } else {
                if (FindNextFileA(
                        (HANDLE)fn_fuji.dir_handle,
                        &fdata)) {
                    if (arg) strcpy((char *)arg,
                        fdata.cFileName);
                    fn_fuji.dir_position++;
                } else {
                    if (arg) ((char *)arg)[0] = '\0';
                    return FN_ERR_EOF;
                }
            }
        }
#elif !defined(__MSDOS__) && !defined(__DOS__)
        {
            struct dirent *ent;
            ent = readdir((DIR *)fn_fuji.dir_handle);
            if (ent) {
                if (arg) strcpy((char *)arg, ent->d_name);
                fn_fuji.dir_position++;
            } else {
                if (arg) ((char *)arg)[0] = '\0';
                return FN_ERR_EOF;
            }
        }
#else
        if (arg) ((char *)arg)[0] = '\0';
        return FN_ERR_NOT_IMPL;
#endif
        return FN_ERR_OK;

    case FNIO_CLOSE_DIRECTORY:
        if (fn_fuji.dir_open && fn_fuji.dir_handle) {
#ifdef _WIN32
            FindClose((HANDLE)fn_fuji.dir_handle);
#elif !defined(__MSDOS__) && !defined(__DOS__)
            closedir((DIR *)fn_fuji.dir_handle);
#endif
            fn_fuji.dir_handle = NULL;
        }
        fn_fuji.dir_open = 0;
        fn_fuji.dir_position = 0;
        return FN_ERR_OK;

    case FNIO_SET_DIR_POSITION:
        if (arg) fn_fuji.dir_position = *(int *)arg;
        return FN_ERR_OK;

    case FNIO_GET_DIR_POSITION:
        if (arg) *(int *)arg = fn_fuji.dir_position;
        return FN_ERR_OK;

    default:
        return FN_ERR_BAD_CMD;
    }
}

 // FUJI: info -- Return adapter information.
static const char *fn_fuji_info(VDev *d, const char *key)
{
    static char buf[64];
    (void)d;

    if (strcmp(key, "version") == 0)
        return fn_fuji.fn_version;
    if (strcmp(key, "ssid") == 0)
        return fn_fuji.ssid;
    if (strcmp(key, "hostname") == 0)
        return fn_fuji.hostname;
    if (strcmp(key, "ip") == 0) {
        sprintf(buf, "%d.%d.%d.%d",
            fn_fuji.local_ip[0], fn_fuji.local_ip[1],
            fn_fuji.local_ip[2], fn_fuji.local_ip[3]);
        return buf;
    }
    if (strcmp(key, "wifi") == 0) {
        switch (fn_fuji.wifi_status) {
        case FN_WIFI_CONNECTED:       return "CONNECTED";
        case FN_WIFI_NO_SSID:         return "NO_SSID";
        case FN_WIFI_CONNECT_FAILED:  return "FAILED";
        case FN_WIFI_CONNECTION_LOST: return "LOST";
        default: return "UNKNOWN";
        }
    }

    return NULL;
}

// ================================================================
 // CLOCK: DEVICE -- VIRTUAL DEVICE CALLBACKS
 //
 // Network time from FujiNet hardware. On desktop, returns
 // the system clock in the requested format.
 // ================================================================ 

 // CLOCK: read -- Return current time.
static int fn_clock_read(VDev *d, void *buf, int len)
{
    time_t now;
    struct tm *tm_ptr;

    (void)d;

    time(&now);
    tm_ptr = localtime(&now);

    switch (fn_clock_state.format) {
    case FN_TIME_BINARY_SIMPLE: {
        // 7 bytes: century, year, month, day, hour, min, sec
        unsigned char *out = (unsigned char *)buf;
        if (len < 7) return -1;
        out[0] = (unsigned char)((tm_ptr->tm_year + 1900) / 100);
        out[1] = (unsigned char)((tm_ptr->tm_year + 1900) % 100);
        out[2] = (unsigned char)(tm_ptr->tm_mon + 1);
        out[3] = (unsigned char)tm_ptr->tm_mday;
        out[4] = (unsigned char)tm_ptr->tm_hour;
        out[5] = (unsigned char)tm_ptr->tm_min;
        out[6] = (unsigned char)tm_ptr->tm_sec;
        return 7;
    }

    case FN_TIME_BINARY_PRODOS: {
        // 4 bytes: ProDOS date/time format
        unsigned char *out = (unsigned char *)buf;
        unsigned int date_word;
        unsigned int time_word;
        if (len < 4) return -1;
        date_word = ((unsigned int)(tm_ptr->tm_year % 100) << 9)
                  | ((unsigned int)(tm_ptr->tm_mon + 1) << 5)
                  | (unsigned int)tm_ptr->tm_mday;
        time_word = ((unsigned int)tm_ptr->tm_hour << 8)
                  | (unsigned int)tm_ptr->tm_min;
        out[0] = (unsigned char)(date_word & 0xFF);
        out[1] = (unsigned char)((date_word >> 8) & 0xFF);
        out[2] = (unsigned char)(time_word & 0xFF);
        out[3] = (unsigned char)((time_word >> 8) & 0xFF);
        return 4;
    }

    case FN_TIME_BINARY_APETIME: {
        // 6 bytes: day, month, year, hour, min, sec
        unsigned char *out = (unsigned char *)buf;
        if (len < 6) return -1;
        out[0] = (unsigned char)tm_ptr->tm_mday;
        out[1] = (unsigned char)(tm_ptr->tm_mon + 1);
        out[2] = (unsigned char)(tm_ptr->tm_year % 100);
        out[3] = (unsigned char)tm_ptr->tm_hour;
        out[4] = (unsigned char)tm_ptr->tm_min;
        out[5] = (unsigned char)tm_ptr->tm_sec;
        return 6;
    }

    case FN_TIME_ISO_STRING:
    default: {
        // ISO 8601 string: YYYY-MM-DDTHH:MM:SS
        char iso[32];
        int iso_len;
        sprintf(iso, "%04d-%02d-%02dT%02d:%02d:%02d",
            tm_ptr->tm_year + 1900,
            tm_ptr->tm_mon + 1,
            tm_ptr->tm_mday,
            tm_ptr->tm_hour,
            tm_ptr->tm_min,
            tm_ptr->tm_sec);
        iso_len = (int)strlen(iso);
        if (iso_len >= len) iso_len = len - 1;
        memcpy(buf, iso, iso_len);
        ((char *)buf)[iso_len] = '\0';
        return iso_len;
    }
    }
}

 // CLOCK: ioctl -- Set time format and timezone.
static int fn_clock_ioctl(VDev *d, int cmd, void *arg)
{
    (void)d;

    switch (cmd) {
    case FNIO_CLOCK_SET_FORMAT:
        if (arg) fn_clock_state.format = *(int *)arg;
        return FN_ERR_OK;

    case FNIO_CLOCK_SET_TZ:
        if (arg) {
            strncpy(fn_clock_state.timezone,
                    (const char *)arg,
                    sizeof(fn_clock_state.timezone) - 1);
        }
        return FN_ERR_OK;

    default:
        return FN_ERR_BAD_CMD;
    }
}

 // CLOCK: info -- Return clock metadata.
static const char *fn_clock_info(VDev *d, const char *key)
{
    (void)d;
    if (strcmp(key, "format") == 0) {
        switch (fn_clock_state.format) {
        case FN_TIME_BINARY_SIMPLE:  return "BINARY_SIMPLE";
        case FN_TIME_BINARY_PRODOS: return "BINARY_PRODOS";
        case FN_TIME_BINARY_APETIME:return "BINARY_APETIME";
        case FN_TIME_ISO_STRING:     return "ISO_STRING";
        default: return "UNKNOWN";
        }
    }
    if (strcmp(key, "timezone") == 0)
        return fn_clock_state.timezone[0]
               ? fn_clock_state.timezone : "LOCAL";
    return NULL;
}

// ================================================================
 // MODULE LIFECYCLE
 // ================================================================ 

 // Module init -- Called when BASIC program issues MODULE "FUJINET".
 //
 // Initializes Winsock, clears channels, configures FUJI state,
 // and registers the three virtual devices.
static int fn_module_init(void *rt)
{
    VDev net_dev;
    VDev fuji_dev;
    VDev clock_dev;
    int i;

    (void)rt;

    // Initialize platform sockets
    if (fn_socket_init() != 0) return -1;

    // Clear all channels
    for (i = 0; i < FN_MAX_CHANNELS; i++) {
        memset(&fn_channels[i], 0, sizeof(FnChannel));
        fn_channels[i].sock_fd = -1;
        fn_channels[i].tnfs_fd = -1;
    }

    // Initialize FUJI state
    fn_fuji_init_config();
    fn_fuji_load_slots();

    // Initialize clock state
    memset(&fn_clock_state, 0, sizeof(fn_clock_state));
    fn_clock_state.format = FN_TIME_ISO_STRING;

    // --- Register N: device ---
    memset(&net_dev, 0, sizeof(net_dev));
    net_dev.name = "N:";
    net_dev.dev_class = VDCLASS_NETWORK;
    net_dev.dev_caps = VDCAP_READ | VDCAP_WRITE |
                       VDCAP_BINARY | VDCAP_DUPLEX |
                       VDCAP_CONTROL | VDCAP_STATUS;
    net_dev.dev_version = "1.0";
    net_dev.dev_description =
        "FujiNet N: network adapter (TCP/UDP/HTTP/TNFS)";
    net_dev.dev_open = fn_net_open;
    net_dev.dev_close = fn_net_close;
    net_dev.dev_putc = fn_net_putc;
    net_dev.dev_puts = fn_net_puts;
    net_dev.dev_getc = fn_net_getc;
    net_dev.dev_gets = fn_net_gets;
    net_dev.dev_read = fn_net_read;
    net_dev.dev_write = fn_net_write;
    net_dev.dev_ioctl = fn_net_ioctl;
    net_dev.dev_status = fn_net_status;
    net_dev.dev_poll = fn_net_poll;
    net_dev.dev_info = fn_net_info;
    net_dev.user_data = (void *)(long)(-1);

    fn_net_vdev_id = vdev_register(&net_dev);
    if (fn_net_vdev_id < 0) return -1;

    // --- Register FUJI: device ---
    memset(&fuji_dev, 0, sizeof(fuji_dev));
    fuji_dev.name = "FUJI:";
    fuji_dev.dev_class = VDCLASS_STORAGE;
    fuji_dev.dev_caps = VDCAP_READ | VDCAP_WRITE |
                        VDCAP_CONTROL | VDCAP_STATUS;
    fuji_dev.dev_version = "1.0";
    fuji_dev.dev_description =
        "FujiNet FUJI: config (WiFi, slots, AppKey)";
    fuji_dev.dev_ioctl = fn_fuji_ioctl;
    fuji_dev.dev_info = fn_fuji_info;

    fn_fuji_vdev_id = vdev_register(&fuji_dev);
    if (fn_fuji_vdev_id < 0) return -1;

    // --- Register CLOCK: device ---
    memset(&clock_dev, 0, sizeof(clock_dev));
    clock_dev.name = "CLOCK:";
    clock_dev.dev_class = VDCLASS_TIMER;
    clock_dev.dev_caps = VDCAP_READ | VDCAP_CONTROL;
    clock_dev.dev_version = "1.0";
    clock_dev.dev_description =
        "FujiNet CLOCK: NTP time (desktop: system clock)";
    clock_dev.dev_read = fn_clock_read;
    clock_dev.dev_ioctl = fn_clock_ioctl;
    clock_dev.dev_info = fn_clock_info;

    fn_clock_vdev_id = vdev_register(&clock_dev);
    if (fn_clock_vdev_id < 0) return -1;

    return 0;
}

 // Module cleanup -- Close all open channels, shut down sockets.
static void fn_module_cleanup(void)
{
    int i;

    // Close any open channels
    for (i = 0; i < FN_MAX_CHANNELS; i++) {
        if (fn_channels[i].in_use) {
            if (fn_channels[i].proto == FN_PROTO_TNFS) {
                if (fn_channels[i].tnfs_fd >= 0)
                    fn_tnfs_close_file(&fn_channels[i]);
                fn_tnfs_unmount(&fn_channels[i]);
            }
            if (fn_channels[i].sock_fd >= 0)
                fn_closesocket(fn_channels[i].sock_fd);
            if (fn_channels[i].http_body)
                free(fn_channels[i].http_body);
            if (fn_channels[i].json_data)
                free(fn_channels[i].json_data);
            memset(&fn_channels[i], 0, sizeof(FnChannel));
            fn_channels[i].sock_fd = -1;
        }
    }

    // Save host slot configuration
    fn_fuji_save_slots();

    // Shut down socket subsystem
    fn_socket_cleanup();

    fn_net_vdev_id = -1;
    fn_fuji_vdev_id = -1;
    fn_clock_vdev_id = -1;
}

// ================================================================
 // MODULE DESCRIPTOR AND REGISTRATION
 // ================================================================ 

static const ModuleInfo fn_module_info = {
    "FUJINET",
    "1.0",
    "FujiNet virtual device (N:, FUJI:, CLOCK:)",
    MOD_DEVICE,
    CAP_NETWORK | CAP_IO,
    fn_module_init,
    fn_module_cleanup
};

void mod_fujinet_register(void)
{
    module_register(&fn_module_info);
}
