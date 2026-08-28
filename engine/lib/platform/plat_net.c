// FILENAME: plat_net.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (select.c, string.h)
// NEEDS: libengine (select.h, string.c, time.h, time.c, vm.h)
// NEEDS: libkernel (types.h)
// NEEDS: libplatform (platform.h)
// Provides cross-platform OS abstraction primitives for plat_net.
//
// ---- Includes ----

#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <signal.h>
#ifndef STANDALONE_EDITOR
#include "vm/vm.h"
#endif

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <conio.h>
    #include <direct.h>
    #include <io.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <dlfcn.h>
    #include <errno.h>
    #include <pthread.h>
    #ifndef STANDALONE_EDITOR
        #include <ncurses.h>
    #endif
#elif defined(__WATCOMC__) || defined(MSDOS)
    #include <dos.h>
    #include <conio.h>
    #include <direct.h>
#endif

#ifndef _WIN32
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

int platform_net_init(void) {
#if defined(_WIN32)
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0 ? 0 : -1;
#else
    return 0;
#endif
}

void platform_net_cleanup(void) {
#if defined(_WIN32)
    WSACleanup();
#endif
}

BppSocket platform_socket_connect(const char *host, int port, int socktype, BppError *err) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (socktype == BASIC_SOCK_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;
    
    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        if (err) { err->code = 57; err->message = "Host resolution failed"; }
        return BASIC_INVALID_SOCKET;
    }
    
    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        if (err) { err->code = 57; err->message = "Socket creation failed"; }
        return BASIC_INVALID_SOCKET;
    }
    
    if (socktype == BASIC_SOCK_STREAM) {
        if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
#if defined(_WIN32)
            closesocket(sock);
#else
            close(sock);
#endif
            freeaddrinfo(res);
            if (err) { err->code = 57; err->message = "Connection failed"; }
            return BASIC_INVALID_SOCKET;
        }
    }
    
    freeaddrinfo(res);
    return (BppSocket)sock;
#else
    (void)host; (void)port; (void)socktype;
    if (err) { err->code = 57; err->message = "Networking not supported on this platform preset"; }
    return BASIC_INVALID_SOCKET;
#endif
}

int platform_socket_send(BppSocket sock, const void *buf, int len) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    return send((SOCKET)sock, buf, len, 0);
#else
    (void)sock; (void)buf; (void)len;
    return -1;
#endif
}

int platform_socket_recv(BppSocket sock, void *buf, int len, int *err_code) {
    if (err_code) *err_code = 0;
#if defined(_WIN32)
    int rec = recv((SOCKET)sock, buf, len, 0);
    if (rec < 0) {
        int wsa_err = WSAGetLastError();
        if (wsa_err == WSAEWOULDBLOCK) {
            if (err_code) *err_code = 1;
        } else {
            if (err_code) *err_code = -1;
        }
    }
    return rec;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    int rec = recv((int)sock, buf, len, 0);
    if (rec < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            if (err_code) *err_code = 1;
        } else {
            if (err_code) *err_code = -1;
        }
    }
    return rec;
#else
    (void)sock; (void)buf; (void)len;
    if (err_code) *err_code = -1;
    return -1;
#endif
}

void platform_socket_close(BppSocket sock) {
#if defined(_WIN32)
    closesocket((SOCKET)sock);
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    close((int)sock);
#else
    (void)sock;
#endif
}

int platform_socket_set_nonblocking(BppSocket sock, int nonblock) {
#if defined(_WIN32)
    u_long mode = nonblock ? 1 : 0;
    return ioctlsocket((SOCKET)sock, FIONBIO, &mode) == 0 ? 0 : -1;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    int flags = fcntl((int)sock, F_GETFL, 0);
    if (flags < 0) return -1;
    if (nonblock) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl((int)sock, F_SETFL, flags) == 0 ? 0 : -1;
#else
    (void)sock; (void)nonblock;
    return -1;
#endif
}

int platform_socket_poll_readable(BppSocket sock, int timeout_ms) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET((SOCKET)sock, &fds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    int sel = select((int)sock + 1, &fds, NULL, NULL, &tv);
    if (sel > 0 && FD_ISSET((SOCKET)sock, &fds)) {
        return 1;
    }
    return 0;
#else
    (void)sock; (void)timeout_ms;
    return 0;
#endif
}

BppSocket platform_socket_listen(int port, BppError *err) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        if (err) { err->code = 57; err->message = "Socket creation failed"; }
        return BASIC_INVALID_SOCKET;
    }

    int opt = 1;
#if defined(_WIN32)
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
#if defined(_WIN32)
        closesocket(sock);
#else
        close(sock);
#endif
        if (err) { err->code = 57; err->message = "Socket bind failed"; }
        return BASIC_INVALID_SOCKET;
    }

    if (listen(sock, 5) == SOCKET_ERROR) {
#if defined(_WIN32)
        closesocket(sock);
#else
        close(sock);
#endif
        if (err) { err->code = 57; err->message = "Socket listen failed"; }
        return BASIC_INVALID_SOCKET;
    }

    return (BppSocket)sock;
#else
    (void)port; if (err) { err->code = 57; err->message = "Sockets not supported"; }
    return BASIC_INVALID_SOCKET;
#endif
}

BppSocket platform_socket_accept(BppSocket listen_sock, char *client_ip_buf, int ip_buf_len, BppError *err) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    struct sockaddr addr;
    socklen_t addr_len = sizeof(addr);
    SOCKET client_sock = accept((SOCKET)listen_sock, &addr, &addr_len);
    if (client_sock == INVALID_SOCKET) {
        return BASIC_INVALID_SOCKET;
    }

    if (client_ip_buf && ip_buf_len > 0) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        const char *ip_str = inet_ntoa(addr_in->sin_addr);
        if (ip_str) {
            strncpy(client_ip_buf, ip_str, ip_buf_len - 1);
            client_ip_buf[ip_buf_len - 1] = '\0';
        } else {
            client_ip_buf[0] = '\0';
        }
    }

    return (BppSocket)client_sock;
#else
    (void)listen_sock; (void)client_ip_buf; (void)ip_buf_len;
    if (err) { err->code = 57; err->message = "Sockets not supported"; }
    return BASIC_INVALID_SOCKET;
#endif
}

