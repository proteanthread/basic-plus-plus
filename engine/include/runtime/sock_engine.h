// FILENAME: sock_engine.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (sock_engine.c)
// NEEDED BY: libengine (func_sock.c, stmt_sock.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Declares low-level BSD socket table, multiplexing and non-blocking I/O.
//
// ---- Includes ----

#ifndef RUNTIME_SOCK_ENGINE_H
#define RUNTIME_SOCK_ENGINE_H

#include "types/types.h"
#include "vm/vm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SOCK_MAX_HANDLES 32
#define SOCK_TYPE_TCP 1
#define SOCK_TYPE_UDP 2
#define SOCK_TYPE_RAW 3

#define SOCK_POLL_IN  1
#define SOCK_POLL_OUT 2
#define SOCK_POLL_ERR 4

typedef struct {
    int  handle;
    int  type;
    int  port;
    bool is_listening;
    bool is_connected;
    int  backlog;
    char remote_ip[64];
    int  remote_port;
    char rx_buffer[1024];
    size_t rx_len;
} SockEntry;

typedef struct {
    SockEntry entries[SOCK_MAX_HANDLES];
    int       active_count;
    bool      initialized;
} SockEngineContext;

BppError sock_engine_init(void);
int      sock_open(int type);
BppError sock_bind(int handle, int port);
BppError sock_listen(int handle, int backlog);
int      sock_accept(int listen_handle, char *out_client_ip, int ip_max, int *out_port);
BppError sock_send(int handle, const char *data, size_t len);
BppError sock_recv(int handle, char *buf, size_t max_len, size_t *out_len, int timeout_ms);
int      sock_poll(int handle, int mask, int timeout_ms);
BppError sock_close(int handle);
BppError sock_setsockopt(int handle, const char *opt, int val);
bool     sock_has_pending_data(int handle);
void     sock_engine_shutdown(void);

#endif // RUNTIME_SOCK_ENGINE_H
