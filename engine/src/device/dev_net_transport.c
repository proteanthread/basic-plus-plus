// FILENAME: dev_net_transport.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c), libkernel (vdev.c)
// NEEDS: libkernel (dev_net_transport.h, vdev.h)
// Direct network transport device drivers (MAC, TCP, UDP, NET).
//
// ---- Includes ----

#include "device/dev_net_transport.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "hal/hal.h"

#define NET_BUFFER_SIZE 4096

typedef struct {
    char protocol[16];
    char host_or_mac[128];
    int  port;
    uint8_t rx_buf[NET_BUFFER_SIZE];
    int  rx_head;
    int  rx_tail;
    bool is_connected;
} NetTransportContext;

static int net_trans_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    (void)c;
    return c;
}

static int net_trans_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    while (*s) {
        if (net_trans_putc(dev, (unsigned char)*s) < 0) return -1;
        s++;
    }
    return 0;
}

static int net_trans_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    NetTransportContext *ctx = (NetTransportContext *)dev->priv;
    if (ctx->rx_head != ctx->rx_tail) {
        uint8_t ch = ctx->rx_buf[ctx->rx_tail];
        ctx->rx_tail = (ctx->rx_tail + 1) % NET_BUFFER_SIZE;
        return (int)ch;
    }
    return -1;
}

static char *net_trans_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size <= 1) return NULL;
    size_t idx = 0;
    while (idx + 1 < size) {
        int c = net_trans_getc(dev);
        if (c < 0) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int net_trans_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int net_trans_cls(VDev *dev) {
    (void)dev;
    return 0;
}

static int net_trans_dev_open(VDev *dev, const char *path, int mode) {
    (void)mode;
    if (!dev || !dev->priv) return -1;
    NetTransportContext *ctx = (NetTransportContext *)dev->priv;
    if (path && *path) {
        runtime_strncpy(ctx->host_or_mac, path, sizeof(ctx->host_or_mac) - 1);
    }
    ctx->is_connected = true;
    return 0;
}

static int net_trans_dev_close(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    NetTransportContext *ctx = (NetTransportContext *)dev->priv;
    ctx->is_connected = false;
    return 0;
}

VDev dev_net_transport_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "TCP:";
    dev.dev_class = VDCLASS_NETWORK;
    dev.dev_version = "6.5.2";
    dev.dev_description = "Direct Network Transport Protocol Stream";
    dev.dev_caps = VDCAP_STREAM | VDCAP_BINARY | VDCAP_ASYNC | VDCAP_DUPLEX;

    dev.ops.putc = net_trans_putc;
    dev.ops.puts = net_trans_puts;
    dev.ops.getc = net_trans_getc;
    dev.ops.gets = net_trans_gets;
    dev.ops.flush = net_trans_flush;
    dev.ops.cls = net_trans_cls;

    dev.dev_open = net_trans_dev_open;
    dev.dev_close = net_trans_dev_close;

    static NetTransportContext s_trans_pool[8];
    static size_t s_trans_pool_count = 0;
    NetTransportContext *ctx = NULL;
    if (s_trans_pool_count < 8) {
        ctx = &s_trans_pool[s_trans_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            ctx = (NetTransportContext *)hal->mem.alloc(sizeof(NetTransportContext));
        }
    }
    if (ctx) {
        runtime_memset(ctx, 0, sizeof(NetTransportContext));
        runtime_strncpy(ctx->protocol, name ? name : "TCP", sizeof(ctx->protocol) - 1);
    }
    dev.priv = ctx;
    return dev;
}
