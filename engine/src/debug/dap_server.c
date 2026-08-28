// FILENAME: dap_server.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libengine (vm.h)
// Provides core logic and interface definitions for dap_server within BASIC++.
//
// ---- Includes ----

#include "vm/vm.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

typedef struct {
    int server_fd;
    int client_fd;
    bool active;
    bool paused;
} DapServerState;

static DapServerState g_dap = { -1, -1, false, false };

static void dap_debug_hook(VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm;
    (void)user_data;
    if (!g_dap.active || g_dap.client_fd < 0) return;

    // Basic JSON-RPC Event payload
    char buffer[512];
    runtime_snprintf(buffer, sizeof(buffer), "{\"type\":\"event\",\"event\":\"%s\",\"body\":{\"line\":%d,\"symbol\":\"%s\"}}\r\n", 
                     event_type, line_num, symbol ? symbol : "");
    
#ifdef _WIN32
    send(g_dap.client_fd, buffer, (int)runtime_strlen(buffer), 0);
#else
    send(g_dap.client_fd, buffer, runtime_strlen(buffer), 0);
#endif
}

void dap_server_init(VMContext *vm) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    g_dap.server_fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (g_dap.server_fd >= 0) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(9000);

        if (bind(g_dap.server_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            listen(g_dap.server_fd, 1);
            g_dap.active = true;
            vm_set_debug_hook(vm, dap_debug_hook, NULL);
        }
    }
}

void dap_server_poll(VMContext *vm) {
    (void)vm;
    if (!g_dap.active || g_dap.server_fd < 0) return;

    // Extremely basic non-blocking accept
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(g_dap.server_fd, FIONBIO, &mode);
#else
    fcntl(g_dap.server_fd, F_SETFL, O_NONBLOCK);
#endif

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int addrlen = sizeof(client_addr);
#else
    socklen_t addrlen = sizeof(client_addr);
#endif

    int new_fd = (int)accept(g_dap.server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (new_fd >= 0) {
        if (g_dap.client_fd >= 0) {
#ifdef _WIN32
            closesocket(g_dap.client_fd);
#else
            close(g_dap.client_fd);
#endif
        }
        g_dap.client_fd = new_fd;
    }
}
