#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "vdev_net.h"
#include "security.h"

int vdev_net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
#endif
    return 0;
}

void vdev_net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static int net_close(VDev *d) {
    if (d && d->user_data) {
        int sock = (int)(long)d->user_data;
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        if (d->name) {
            free((void*)d->name);
        }
        free(d);
    }
    return 0;
}

static int net_read(VDev *d, void *buf, int len) {
    int sock = (int)(long)d->user_data;
    int r = recv(sock, (char *)buf, len, 0);
    return r > 0 ? r : -1;
}

static int net_write(VDev *d, const void *buf, int len) {
    int sock = (int)(long)d->user_data;
    int r = send(sock, (const char *)buf, len, 0);
    return r > 0 ? r : -1;
}

static int net_putc(VDev *d, int ch) {
    char c = (char)ch;
    return net_write(d, &c, 1) == 1 ? ch : -1;
}

static int net_getc(VDev *d) {
    char c;
    if (net_read(d, &c, 1) == 1) return (unsigned char)c;
    return -1;
}

static int net_puts(VDev *d, const char *s) {
    int len = (int)strlen(s);
    return net_write(d, s, len) == len ? 0 : -1;
}

static int net_gets(VDev *d, char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        int c = net_getc(d);
        if (c == -1) break;
        buf[i++] = (char)c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return i > 0 ? 0 : -1;
}

VDev *vdev_net_open(const char *uri) {
    char proto[4], host[256], port[32];
    struct addrinfo hints, *res;
    int sock = -1;
    VDev *d;
    char *name_copy;
    size_t uri_len;
    
    if (sscanf(uri, "%3[^:]:%255[^:]:%31s", proto, host, port) != 3) {
        return NULL;
    }

    if (security_check(SECOP_NETWORK, 0) != 0) {
        return NULL;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;

    if (strcmp(proto, "TCP") == 0) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    } else if (strcmp(proto, "UDP") == 0) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    } else {
        return NULL;
    }

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return NULL;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return NULL;
    }

    if (connect(sock, res->ai_addr, (int)res->ai_addrlen) < 0) {
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
    d->dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_BINARY | VDCAP_STREAM;
    d->user_data = (void *)(long)sock;
    d->dev_close = net_close;
    d->dev_read = net_read;
    d->dev_write = net_write;
    d->dev_putc = net_putc;
    d->dev_getc = net_getc;
    d->dev_puts = net_puts;
    d->dev_gets = net_gets;

    return d;
}
