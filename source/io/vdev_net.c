/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vdev_net.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - vdev_net.c
 // ---
 //
 // Core network VDev -- provides TCP/UDP socket access as a
 // virtual device, always available (no MODULE required).
 //
 // Device prefix:
 //   NET:  -- Core virtual network (always available)
 //   N:   -- FujiNet only (requires MODULE "FUJINET")
 //
 // Supports three URI formats:
 //   Legacy:   TCP:host:port   /  UDP:host:port
 //   Core:     NET:PROTO://host:port/path
 //   FujiNet:  N:PROTO://host:port/path
 //
 // Supported protocols and default ports:
 //   TCP(0)  UDP(0)  HTTP(80)  HTTPS(443)  FTP(21)
 //   SFTP(22)  SSH(22)  TELNET(23)  IRC(6667)
 //   SMTP(25)  POP3(110)  IMAP(143)  NNTP(119)
 //   SNMP(161)  TNFS(16384)
 //
 // Port access is gated by security_check_port().
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#ifndef _WIN32
  #if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200112L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define TokenType WinTokenType
#include <winsock2.h>
#include <ws2tcpip.h>
#undef TokenType
#pragma comment(lib, "ws2_32.lib")
#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
#define VNET_NO_NETWORKING 1
typedef int SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "vdev_net.h"
#include "../security.h"
#include "../console.h"
#include "../boot.h"

// ================================================================
 // Protocol-to-port mapping
 // ================================================================ 

typedef struct ProtoMap {
    const char *name;
    int default_port;
    int use_tcp; // 1=TCP, 0=UDP
} ProtoMap;

static const ProtoMap proto_table[] = {
    { "TCP",    0,     1 },
    { "UDP",    0,     0 },
    { "HTTP",   80,    1 },
    { "HTTPS",  443,   1 },
    { "FTP",    21,    1 },
    { "SFTP",   22,    1 },
    { "SSH",    22,    1 },
    { "TELNET", 23,    1 },
    { "IRC",    6667,  1 },
    { "SMTP",   25,    1 },
    { "POP3",   110,   1 },
    { "IMAP",   143,   1 },
    { "NNTP",   119,   1 },
    { "SNMP",   161,   0 },
    { "TNFS",   16384, 0 },
    { NULL,     0,     0 }
};

// Case-insensitive compare for protocol names
static int proto_eq(const char *a, const char *b)
{
    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

// Look up a protocol by name
static const ProtoMap *proto_lookup(const char *name)
{
    int i;
    for (i = 0; proto_table[i].name != NULL; i++) {
        if (proto_eq(proto_table[i].name, name))
            return &proto_table[i];
    }
    return NULL;
}

// ================================================================
 // URL PARSER
 //
 // Parses both formats:
 //   TCP:host:port           -> proto="TCP", host, port
 //   N:PROTO://host:port/    -> proto="PROTO", host, port
 // ================================================================ 

static int parse_net_uri(const char *uri,
    char *proto, int proto_max,
    char *host, int host_max,
    char *port, int port_max)
{
    const char *p = uri;
    const char *pp;
    int plen, hlen;

    // Skip "NET:" or "N:" prefix if present
    if ((p[0] == 'N' || p[0] == 'n') &&
        (p[1] == 'E' || p[1] == 'e') &&
        (p[2] == 'T' || p[2] == 't') &&
         p[3] == ':')
        p += 4;
    else if ((p[0] == 'N' || p[0] == 'n') && p[1] == ':')
        p += 2;

    // Check for "PROTO://host:port" URL format
    pp = strstr(p, "://");
    if (pp != NULL) {
        // Extract protocol
        plen = (int)(pp - p);
        if (plen <= 0 || plen >= proto_max) return -1;
        memcpy(proto, p, (size_t)plen);
        proto[plen] = '\0';

        // Skip "://"
        p = pp + 3;

        // Extract host (up to ':' or '/' or end)
        pp = p;
        while (*pp && *pp != ':' && *pp != '/')
            pp++;
        hlen = (int)(pp - p);
        if (hlen <= 0 || hlen >= host_max) return -1;
        memcpy(host, p, (size_t)hlen);
        host[hlen] = '\0';

        // Extract port (optional)
        port[0] = '\0';
        if (*pp == ':') {
            int pi = 0;
            pp++;
            while (*pp >= '0' && *pp <= '9' &&
                   pi < port_max - 1) {
                port[pi++] = *pp++;
            }
            port[pi] = '\0';
        }

        return 0;
    }

    // Legacy format: PROTO:host:port
    if (sscanf(uri, "%3[^:]:%255[^:]:%31s",
               proto, host, port) == 3) {
        return 0;
    }

    return -1;
}

// ================================================================
 // PLATFORM INIT / CLEANUP
 // ================================================================ 

static int s_net_initialized = 0;
static int s_net_init_result = 0;

int vdev_net_init(void)
{
    if (s_net_initialized) return s_net_init_result;
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        s_net_init_result = -1;
        s_net_initialized = 1;
        return -1;
    }
#endif
    s_net_init_result = 0;
    s_net_initialized = 1;
    return 0;
}

void vdev_net_cleanup(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

// ================================================================
 // VDEV CALLBACKS
 // ================================================================ 

static int net_close(VDev *d)
{
    if (d && d->user_data) {
        int sock = (int)(intptr_t)d->user_data;
#ifdef _WIN32
        closesocket(sock);
#elif !defined(VNET_NO_NETWORKING)
        close(sock);
#endif
        if (d->name) {
            free((void*)d->name);
        }
        free(d);
    }
    return 0;
}

static int net_read(VDev *d, void *buf, int len)
{
#ifdef VNET_NO_NETWORKING
    (void)d; (void)buf; (void)len;
    return -1;
#else
    int sock = (int)(intptr_t)d->user_data;
    int r = recv(sock, (char *)buf, len, 0);
    return r > 0 ? r : -1;
#endif
}

static int net_write(VDev *d, const void *buf, int len)
{
#ifdef VNET_NO_NETWORKING
    (void)d; (void)buf; (void)len;
    return -1;
#else
    int sock = (int)(intptr_t)d->user_data;
    int r = send(sock, (const char *)buf, len, 0);
    return r > 0 ? r : -1;
#endif
}

static int net_putc(VDev *d, int ch)
{
    char c = (char)ch;
    return net_write(d, &c, 1) == 1 ? ch : -1;
}

static int net_getc(VDev *d)
{
    char c;
    if (net_read(d, &c, 1) == 1) return (unsigned char)c;
    return -1;
}

static int net_puts(VDev *d, const char *s)
{
    int len = (int)strlen(s);
    return net_write(d, s, len) == len ? 0 : -1;
}

static int net_gets(VDev *d, char *buf, int max)
{
    int i = 0;
    while (i < max - 1) {
        int c = net_getc(d);
        if (c == -1) break;
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    return i > 0 ? 0 : -1;
}

static int net_status(VDev *d)
{
    (void)d;
    return 0; // connected
}

// ================================================================
 // VDEV_NET_OPEN -- Main entry point
 // ================================================================ 

VDev *vdev_net_open(const char *uri)
{
#ifdef VNET_NO_NETWORKING
    (void)uri;
    return NULL;
#else
    if (vdev_net_init() != 0) {
        boot_downgrade_status(BOOT_DEGRADED);
        return NULL;
    }

    char proto[16], host[256], port_str[32];
    const ProtoMap *pm;
    struct addrinfo hints, *res;
    int sock = -1;
    int port_num;
    VDev *d;
    char *name_copy;
    size_t uri_len;

    if (parse_net_uri(uri, proto, sizeof(proto),
                      host, sizeof(host),
                      port_str, sizeof(port_str)) != 0) {
        return NULL;
    }

    // Look up protocol
    pm = proto_lookup(proto);
    if (pm == NULL) {
        printf("Unknown network protocol: %s\n", proto);
        return NULL;
    }

    // Resolve port
    if (port_str[0] != '\0') {
        port_num = atoi(port_str);
    } else {
        port_num = pm->default_port;
    }

    if (port_num <= 0 || port_num > 65535) {
        printf("Invalid port for %s\n", proto);
        return NULL;
    }

    // Security: check SECOP_NETWORK
    if (security_check(SECOP_NETWORK, 0) != 0) {
        return NULL;
    }

    // Security: check port
    if (security_check_port(port_num, 0) != 0) {
        return NULL;
    }

    // Format port string for getaddrinfo
    sprintf(port_str, "%d", port_num);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (pm->use_tcp) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    } else {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        printf("DNS resolution failed: %s\n", host);
        return NULL;
    }

    sock = (int)socket(res->ai_family, res->ai_socktype,
                       res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return NULL;
    }

    if (connect(sock, res->ai_addr,
                (int)res->ai_addrlen) < 0) {
        printf("Connection refused: %s:%d\n",
               host, port_num);
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        freeaddrinfo(res);
        return NULL;
    }

    freeaddrinfo(res);

    d = (VDev *)malloc(sizeof(VDev));
    if (!d) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return NULL;
    }
    memset(d, 0, sizeof(VDev));

    uri_len = strlen(uri);
    name_copy = (char *)malloc(uri_len + 1);
    if (name_copy) {
        strcpy(name_copy, uri);
    }
    d->name = name_copy;

    d->dev_class = VDCLASS_NETWORK;
    d->dev_caps = VDCAP_READ | VDCAP_WRITE |
                  VDCAP_BINARY | VDCAP_STREAM;
    d->user_data = (void *)(intptr_t)sock;
    d->dev_close = net_close;
    d->dev_read = net_read;
    d->dev_write = net_write;
    d->dev_putc = net_putc;
    d->dev_getc = net_getc;
    d->dev_puts = net_puts;
    d->dev_gets = net_gets;
    d->dev_status = net_status;

    return d;
#endif // VNET_NO_NETWORKING
}

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Dynamic OpenSSL types
typedef void* SSL_CTX_PTR;
typedef void* SSL_PTR;
typedef void* SSL_METHOD_PTR;

static void* ssl_lib = NULL;
static void* crypto_lib = NULL;

static int tls_initialized = 0;

// OpenSSL function pointers
static void (*p_SSL_library_init)(void) = NULL;
//static void (*p_OpenSSL_add_all_algorithms)(void) = NULL;
//static void (*p_SSL_load_error_strings)(void) = NULL;
static SSL_METHOD_PTR (*p_TLS_client_method)(void) = NULL;
static SSL_CTX_PTR (*p_SSL_CTX_new)(SSL_METHOD_PTR) = NULL;
static SSL_PTR (*p_SSL_new)(SSL_CTX_PTR) = NULL;
static int (*p_SSL_set_fd)(SSL_PTR, int) = NULL;
static int (*p_SSL_connect)(SSL_PTR) = NULL;
static int (*p_SSL_write)(SSL_PTR, const void*, int) = NULL;
static int (*p_SSL_read)(SSL_PTR, void*, int) = NULL;
static void (*p_SSL_free)(SSL_PTR) = NULL;
static void (*p_SSL_CTX_free)(SSL_CTX_PTR) = NULL;

static int init_tls_dynamic(void) {
    if (tls_initialized) return 1;
#ifdef USE_OPENSSL
    // Compile-time linkage takes precedence if defined
    return 1;
#endif

#ifdef _WIN32
    ssl_lib = LoadLibraryA("libssl-1_1-x64.dll");
    if (!ssl_lib) ssl_lib = LoadLibraryA("ssleay32.dll");
    crypto_lib = LoadLibraryA("libcrypto-1_1-x64.dll");
    if (!crypto_lib) crypto_lib = LoadLibraryA("libeay32.dll");
    
    if (ssl_lib && crypto_lib) {
        p_SSL_library_init = (void*)GetProcAddress(ssl_lib, "SSL_library_init");
        p_TLS_client_method = (void*)GetProcAddress(ssl_lib, "TLS_client_method");
        if(!p_TLS_client_method) p_TLS_client_method = (void*)GetProcAddress(ssl_lib, "SSLv23_client_method");
        p_SSL_CTX_new = (void*)GetProcAddress(ssl_lib, "SSL_CTX_new");
        p_SSL_new = (void*)GetProcAddress(ssl_lib, "SSL_new");
        p_SSL_set_fd = (void*)GetProcAddress(ssl_lib, "SSL_set_fd");
        p_SSL_connect = (void*)GetProcAddress(ssl_lib, "SSL_connect");
        p_SSL_write = (void*)GetProcAddress(ssl_lib, "SSL_write");
        p_SSL_read = (void*)GetProcAddress(ssl_lib, "SSL_read");
        p_SSL_free = (void*)GetProcAddress(ssl_lib, "SSL_free");
        p_SSL_CTX_free = (void*)GetProcAddress(ssl_lib, "SSL_CTX_free");
    }
#else
    ssl_lib = dlopen("libssl.so", RTLD_LAZY);
    crypto_lib = dlopen("libcrypto.so", RTLD_LAZY);
    if (ssl_lib && crypto_lib) {
        p_SSL_library_init = dlsym(ssl_lib, "SSL_library_init");
        p_TLS_client_method = dlsym(ssl_lib, "TLS_client_method");
        if(!p_TLS_client_method) p_TLS_client_method = dlsym(ssl_lib, "SSLv23_client_method");
        p_SSL_CTX_new = dlsym(ssl_lib, "SSL_CTX_new");
        p_SSL_new = dlsym(ssl_lib, "SSL_new");
        p_SSL_set_fd = dlsym(ssl_lib, "SSL_set_fd");
        p_SSL_connect = dlsym(ssl_lib, "SSL_connect");
        p_SSL_write = dlsym(ssl_lib, "SSL_write");
        p_SSL_read = dlsym(ssl_lib, "SSL_read");
        p_SSL_free = dlsym(ssl_lib, "SSL_free");
        p_SSL_CTX_free = dlsym(ssl_lib, "SSL_CTX_free");
    }
#endif

    if (p_SSL_CTX_new && p_SSL_new && p_SSL_connect) {
        if (p_SSL_library_init) p_SSL_library_init();
        tls_initialized = 1;
        return 1;
    }
    return 0;
}

// Dummy fetch implementations for now
char *net_gemini_fetch(const char *url) {
    if (!init_tls_dynamic()) {
        // Fallback to proxy
        char *buf = malloc(256);
        snprintf(buf, 256, "PROXY_FALLBACK: GEMINI URL %s", url);
        return buf;
    }
    char *buf = malloc(256);
    snprintf(buf, 256, "TLS_FETCH: GEMINI URL %s", url);
    return buf;
}

char *net_gopher_fetch(const char *url) {
    // Gopher is plaintext TCP
    char *buf = malloc(256);
    snprintf(buf, 256, "GOPHER_FETCH: URL %s", url);
    return buf;
}
