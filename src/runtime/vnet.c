/**
 * @file vnet.c
 * @brief Virtual Network Stack & Sockets (VNet) implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements cross-platform socket management using platform abstractions.
 * - Why it exists: Bridges VM network statements and virtual devices to actual host networks.
 * - Why it works this way: It uses non-blocking or select-guarded socket operations to
 *   prevent execution hangs.
 */

#include "bpp_vnet.h"
#include "bpp_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    BppSocket    sock;
    char         protocol[16];
    char         host[256];
    int          port;
    bool         connected;
    int          last_http_status;
} BppNetChannel;

struct VNetContext {
    MemoryContext *mem;
    BppNetChannel  channels[VNET_MAX_CHANNELS];
    bool           initialized;
};

static void platform_net_init_local(VNetContext *ctx) {
    if (platform_net_init() == 0) {
        ctx->initialized = true;
    }
}

static void platform_net_cleanup_local(VNetContext *ctx) {
    if (ctx->initialized) {
        platform_net_cleanup();
    }
    ctx->initialized = false;
}

static VNetContext *g_vnet_ctx = NULL;

VNetContext *vnet_init(MemoryContext *mem) {
    if (!mem) return NULL;
    VNetContext *ctx = (VNetContext *)malloc(sizeof(VNetContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->initialized = false;
    for (int i = 0; i < VNET_MAX_CHANNELS; ++i) {
        ctx->channels[i].sock = BPP_INVALID_SOCKET;
        ctx->channels[i].connected = false;
        ctx->channels[i].protocol[0] = '\0';
        ctx->channels[i].host[0] = '\0';
        ctx->channels[i].port = 0;
        ctx->channels[i].last_http_status = 0;
    }
    platform_net_init_local(ctx);
    g_vnet_ctx = ctx;
    return ctx;
}

void vnet_shutdown(VNetContext *ctx) {
    if (ctx) {
        if (g_vnet_ctx == ctx) {
            g_vnet_ctx = NULL;
        }
        for (int i = 0; i < VNET_MAX_CHANNELS; ++i) {
            vnet_close(ctx, i);
        }
        platform_net_cleanup_local(ctx);
        free(ctx);
    }
}

BppError vnet_open(VNetContext *ctx, int channel, const char *protocol, const char *host, int port) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx) {
        err.code = 57;
        err.message = "Network subsystem not initialized";
        return err;
    }

    if (channel < 0 || channel >= VNET_MAX_CHANNELS) {
        err.code = 52; /* Bad file number */
        err.message = "Bad channel number";
        return err;
    }

    if (ctx->channels[channel].sock != BPP_INVALID_SOCKET) {
        err.code = 55; /* File already open */
        err.message = "Network channel already open";
        return err;
    }

    int socktype = (strcmp(protocol, "UDP") == 0) ? BPP_SOCK_DGRAM : BPP_SOCK_STREAM;
    BppSocket sock = platform_socket_connect(host, port, socktype, &err);
    if (sock == BPP_INVALID_SOCKET) {
        return err;
    }

    platform_socket_set_nonblocking(sock, 1);

    ctx->channels[channel].sock = sock;
    ctx->channels[channel].connected = true;
    strncpy(ctx->channels[channel].protocol, protocol, 15);
    ctx->channels[channel].protocol[15] = '\0';
    strncpy(ctx->channels[channel].host, host, 255);
    ctx->channels[channel].host[255] = '\0';
    ctx->channels[channel].port = port;
    ctx->channels[channel].last_http_status = 200; /* mock success */

    return err;
}

BppError vnet_open_host(VNetContext *ctx, int channel, int port) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx) {
        err.code = 57;
        err.message = "Network subsystem not initialized";
        return err;
    }

    if (channel < 0 || channel >= VNET_MAX_CHANNELS) {
        err.code = 52;
        err.message = "Bad channel number";
        return err;
    }

    if (ctx->channels[channel].sock != BPP_INVALID_SOCKET) {
        err.code = 55;
        err.message = "Network channel already open";
        return err;
    }

    BppSocket sock = platform_socket_listen(port, &err);
    if (sock == BPP_INVALID_SOCKET) {
        return err;
    }

    platform_socket_set_nonblocking(sock, 1);

    ctx->channels[channel].sock = sock;
    ctx->channels[channel].connected = true;
    strcpy(ctx->channels[channel].protocol, "TCP-LISTEN");
    ctx->channels[channel].host[0] = '\0';
    ctx->channels[channel].port = port;

    return err;
}

BppError vnet_accept(VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx) {
        err.code = 57;
        err.message = "Network subsystem not initialized";
        return err;
    }

    if (listen_channel < 0 || listen_channel >= VNET_MAX_CHANNELS ||
        client_channel < 0 || client_channel >= VNET_MAX_CHANNELS) {
        err.code = 52;
        err.message = "Bad channel number";
        return err;
    }

    if (ctx->channels[listen_channel].sock == BPP_INVALID_SOCKET) {
        err.code = 52;
        err.message = "Listening channel not open";
        return err;
    }

    if (ctx->channels[client_channel].sock != BPP_INVALID_SOCKET) {
        err.code = 55;
        err.message = "Client channel already open";
        return err;
    }

    BppSocket listen_sock = ctx->channels[listen_channel].sock;
    BppSocket client_sock = platform_socket_accept(listen_sock, client_ip_buf, ip_buf_len, &err);
    if (client_sock == BPP_INVALID_SOCKET) {
        err.code = 0; /* No incoming connection at this moment (non-blocking) */
        return err;
    }

    platform_socket_set_nonblocking(client_sock, 1);

    ctx->channels[client_channel].sock = client_sock;
    ctx->channels[client_channel].connected = true;
    strcpy(ctx->channels[client_channel].protocol, "TCP");
    if (client_ip_buf) {
        strncpy(ctx->channels[client_channel].host, client_ip_buf, 255);
        ctx->channels[client_channel].host[255] = '\0';
    } else {
        ctx->channels[client_channel].host[0] = '\0';
    }
    ctx->channels[client_channel].port = ctx->channels[listen_channel].port;

    return err;
}

BppError vnet_send(VNetContext *ctx, int channel, const char *data, size_t len) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS || ctx->channels[channel].sock == BPP_INVALID_SOCKET) {
        err.code = 52;
        err.message = "Channel not open";
        return err;
    }

    BppSocket sock = ctx->channels[channel].sock;
    int sent = platform_socket_send(sock, data, (int)len);
    if (sent < 0) {
        err.code = 57;
        err.message = "Data transmission failed";
    }

    return err;
}

BppError vnet_recv(VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (out_len) *out_len = 0;

    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS || ctx->channels[channel].sock == BPP_INVALID_SOCKET) {
        err.code = 52;
        err.message = "Channel not open";
        return err;
    }

    BppSocket sock = ctx->channels[channel].sock;

    if (platform_socket_poll_readable(sock, 50)) {
        int err_code = 0;
        int rec = platform_socket_recv(sock, buf, (int)max_len, &err_code);
        if (rec > 0) {
            if ((size_t)rec < max_len) {
                buf[rec] = '\0';
            }
            if (out_len) *out_len = (size_t)rec;
        } else if (rec == 0) {
            ctx->channels[channel].connected = false;
        } else {
            if (err_code != 1) { /* not try-again / would-block */
                err.code = 57;
                err.message = "Receive failed";
            }
        }
    }

    return err;
}

void vnet_close(VNetContext *ctx, int channel) {
    if (!ctx) ctx = g_vnet_ctx;
    if (ctx && channel >= 0 && channel < VNET_MAX_CHANNELS) {
        if (ctx->channels[channel].sock != BPP_INVALID_SOCKET) {
            platform_socket_close(ctx->channels[channel].sock);
            ctx->channels[channel].sock = BPP_INVALID_SOCKET;
            ctx->channels[channel].connected = false;
        }
    }
}

int vnet_status(VNetContext *ctx, int channel) {
    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS) return 0;
    return (ctx->channels[channel].sock != BPP_INVALID_SOCKET) ? 1 : 0;
}

bool vnet_connected(VNetContext *ctx, int channel) {
    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS) return false;
    return ctx->channels[channel].connected;
}

const char *vnet_address(VNetContext *ctx, int channel) {
    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS) return "";
    return ctx->channels[channel].host;
}

int vnet_http_status(VNetContext *ctx, int channel) {
    if (!ctx) ctx = g_vnet_ctx;
    if (!ctx || channel < 0 || channel >= VNET_MAX_CHANNELS) return 0;
    return ctx->channels[channel].last_http_status;
}

/* VDev creation helper */
static int net_vdev_putc(VDev *dev, int c) {
    VNetContext *ctx = (VNetContext *)dev->priv;
    char ch = (char)c;
    vnet_send(ctx, 0, &ch, 1);
    return c;
}

static int net_vdev_puts(VDev *dev, const char *s) {
    VNetContext *ctx = (VNetContext *)dev->priv;
    vnet_send(ctx, 0, s, strlen(s));
    return 0;
}

static int net_vdev_getc(VDev *dev) {
    VNetContext *ctx = (VNetContext *)dev->priv;
    char ch;
    size_t out_len = 0;
    BppError err = vnet_recv(ctx, 0, &ch, 1, &out_len);
    if (err.code == 0 && out_len > 0) {
        return (unsigned char)ch;
    }
    return -1;
}

VDev vnet_create_vdev(VNetContext *ctx, const char *name, const char *protocol, const char *host, int port) {
    VDev dev;
    memset(&dev, 0, sizeof(dev));
    dev.name = name;
    dev.dev_class = VDCLASS_CUSTOM;
    dev.dev_version = "1.0";
    dev.dev_description = "Virtual Network Tunnel Device";
    dev.ops.putc = net_vdev_putc;
    dev.ops.puts = net_vdev_puts;
    dev.ops.getc = net_vdev_getc;
    dev.priv = ctx;

    /* Automatically open channel 0 for this VDev */
    vnet_open(ctx, 0, protocol, host, port);

    return dev;
}
