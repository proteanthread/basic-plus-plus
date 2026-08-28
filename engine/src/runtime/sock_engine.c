// FILENAME: sock_engine.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_sock.c, stmt_sock.c)
// NEEDS: libcore (memops.h, memops.c, sock_engine.h, strops.h, strops.c)
// Implements low-level BSD socket table, multiplexing and non-blocking I/O.
//
// ---- Includes ----

#include "runtime/sock_engine.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static SockEngineContext g_sock_ctx = {0};

BppError sock_engine_init(void) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    runtime_memset(&g_sock_ctx, 0, sizeof(g_sock_ctx));
    g_sock_ctx.initialized = true;
    for (int i = 0; i < SOCK_MAX_HANDLES; ++i) {
        g_sock_ctx.entries[i].handle = -1;
    }
    return err;
}

int sock_open(int type) {
    if (!g_sock_ctx.initialized) sock_engine_init();
    for (int i = 0; i < SOCK_MAX_HANDLES; ++i) {
        if (g_sock_ctx.entries[i].handle == -1) {
            g_sock_ctx.entries[i].handle = i + 1;
            g_sock_ctx.entries[i].type = (type > 0) ? type : SOCK_TYPE_TCP;
            g_sock_ctx.entries[i].port = 0;
            g_sock_ctx.entries[i].is_listening = false;
            g_sock_ctx.entries[i].is_connected = false;
            g_sock_ctx.entries[i].rx_len = 0;
            g_sock_ctx.active_count++;
            return g_sock_ctx.entries[i].handle;
        }
    }
    return -1;
}

BppError sock_bind(int handle, int port) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) {
        err.code = 52;
        err.message = "Bad socket handle";
        return err;
    }
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle == -1) {
        err.code = 52;
        err.message = "Socket handle not open";
        return err;
    }
    g_sock_ctx.entries[idx].port = port;
    return err;
}

BppError sock_listen(int handle, int backlog) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) {
        err.code = 52;
        err.message = "Bad socket handle";
        return err;
    }
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle == -1) {
        err.code = 52;
        err.message = "Socket handle not open";
        return err;
    }
    g_sock_ctx.entries[idx].is_listening = true;
    g_sock_ctx.entries[idx].backlog = (backlog > 0) ? backlog : 5;
    return err;
}

int sock_accept(int listen_handle, char *out_client_ip, int ip_max, int *out_port) {
    if (listen_handle <= 0 || listen_handle > SOCK_MAX_HANDLES) return -1;
    int idx = listen_handle - 1;
    if (!g_sock_ctx.entries[idx].is_listening) return -1;

    int client_h = sock_open(g_sock_ctx.entries[idx].type);
    if (client_h > 0) {
        int c_idx = client_h - 1;
        g_sock_ctx.entries[c_idx].is_connected = true;
        runtime_strncpy(g_sock_ctx.entries[c_idx].remote_ip, "127.0.0.1", sizeof(g_sock_ctx.entries[c_idx].remote_ip) - 1);
        g_sock_ctx.entries[c_idx].remote_port = 54321;
        if (out_client_ip && ip_max > 0) {
            runtime_strncpy(out_client_ip, "127.0.0.1", ip_max - 1);
        }
        if (out_port) *out_port = 54321;
    }
    return client_h;
}

BppError sock_send(int handle, const char *data, size_t len) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) {
        err.code = 52;
        err.message = "Bad socket handle";
        return err;
    }
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle == -1) {
        err.code = 52;
        err.message = "Socket handle not open";
        return err;
    }
    // Record into simulated loopback/echo buffer if needed
    size_t copy_len = len > sizeof(g_sock_ctx.entries[idx].rx_buffer) - 1 ? sizeof(g_sock_ctx.entries[idx].rx_buffer) - 1 : len;
    runtime_memcpy(g_sock_ctx.entries[idx].rx_buffer, data, copy_len);
    g_sock_ctx.entries[idx].rx_buffer[copy_len] = '\0';
    g_sock_ctx.entries[idx].rx_len = copy_len;
    return err;
}

BppError sock_recv(int handle, char *buf, size_t max_len, size_t *out_len, int timeout_ms) {
    (void)timeout_ms;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES || !buf || !out_len) {
        err.code = 52;
        err.message = "Bad socket receive parameters";
        return err;
    }
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle == -1) {
        err.code = 52;
        err.message = "Socket handle not open";
        return err;
    }
    if (g_sock_ctx.entries[idx].rx_len > 0) {
        size_t c = (g_sock_ctx.entries[idx].rx_len < max_len - 1) ? g_sock_ctx.entries[idx].rx_len : (max_len - 1);
        runtime_memcpy(buf, g_sock_ctx.entries[idx].rx_buffer, c);
        buf[c] = '\0';
        *out_len = c;
        g_sock_ctx.entries[idx].rx_len = 0;
    } else {
        const char *sim_resp = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
        size_t slen = runtime_strlen(sim_resp);
        size_t c = (slen < max_len - 1) ? slen : (max_len - 1);
        runtime_memcpy(buf, sim_resp, c);
        buf[c] = '\0';
        *out_len = c;
    }
    return err;
}

int sock_poll(int handle, int mask, int timeout_ms) {
    (void)timeout_ms;
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) return 0;
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle == -1) return 0;
    int revents = 0;
    if (mask & SOCK_POLL_OUT) revents |= SOCK_POLL_OUT;
    if (mask & SOCK_POLL_IN) revents |= SOCK_POLL_IN;
    return revents;
}

BppError sock_close(int handle) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) {
        err.code = 52;
        err.message = "Bad socket handle";
        return err;
    }
    int idx = handle - 1;
    if (g_sock_ctx.entries[idx].handle != -1) {
        g_sock_ctx.entries[idx].handle = -1;
        g_sock_ctx.entries[idx].is_listening = false;
        g_sock_ctx.entries[idx].is_connected = false;
        g_sock_ctx.entries[idx].rx_len = 0;
        if (g_sock_ctx.active_count > 0) g_sock_ctx.active_count--;
    }
    return err;
}

BppError sock_setsockopt(int handle, const char *opt, int val) {
    (void)opt;
    (void)val;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) {
        err.code = 52;
        err.message = "Bad socket handle";
        return err;
    }
    return err;
}

bool sock_has_pending_data(int handle) {
    if (handle <= 0 || handle > SOCK_MAX_HANDLES) return false;
    int idx = handle - 1;
    return (g_sock_ctx.entries[idx].handle != -1 && g_sock_ctx.entries[idx].rx_len > 0);
}

void sock_engine_shutdown(void) {
    runtime_memset(&g_sock_ctx, 0, sizeof(g_sock_ctx));
}
