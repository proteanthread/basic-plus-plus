// FILENAME: dev_tandy.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_tandy.h, vdev.h)
// Implementation for TRS-80 and Tandy family peripherals (MDMn:, TELn:, LCDn:, BARn:, WNDn:, LPn:, FDDn:).
//
// ---- Includes ----

#include "device/dev_tandy.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "runtime/memory/alloc.h"

#define TANDY_BUF_SIZE 2048

typedef struct {
    char name[32];
    VDevClass dev_class;
    char buffer[TANDY_BUF_SIZE];
    int head;
    int tail;
    int count;
    int baud;
    bool online;
} TandyContext;

static TandyContext s_tandy_instances[16];
static int s_tandy_count = 0;

static TandyContext *tandy_find_or_create(const char *name) {
    if (!name || !name[0]) name = "MDM1:";
    for (int i = 0; i < s_tandy_count; i++) {
        if (runtime_strcasecmp(s_tandy_instances[i].name, name) == 0) {
            return &s_tandy_instances[i];
        }
    }
    if (s_tandy_count < 16) {
        TandyContext *ctx = &s_tandy_instances[s_tandy_count++];
        runtime_memset(ctx, 0, sizeof(*ctx));
        runtime_snprintf(ctx->name, sizeof(ctx->name), "%s", name);
        ctx->baud = 300;
        ctx->online = true;

        if (runtime_strncasecmp(name, "MDM", 3) == 0 || runtime_strncasecmp(name, "TEL", 3) == 0) {
            ctx->dev_class = VDCLASS_MODEM;
        } else if (runtime_strncasecmp(name, "BAR", 3) == 0) {
            ctx->dev_class = VDCLASS_BARCODE;
        } else if (runtime_strncasecmp(name, "WND", 3) == 0) {
            ctx->dev_class = VDCLASS_WINDOW;
        } else if (runtime_strncasecmp(name, "LCD", 3) == 0) {
            ctx->dev_class = VDCLASS_DISPLAY;
        } else if (runtime_strncasecmp(name, "LP", 2) == 0) {
            ctx->dev_class = VDCLASS_PRINTER;
        } else if (runtime_strncasecmp(name, "FDD", 3) == 0) {
            ctx->dev_class = VDCLASS_STORAGE;
        } else {
            ctx->dev_class = VDCLASS_SERIAL;
        }
        return ctx;
    }
    return &s_tandy_instances[0];
}

static int tandy_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    TandyContext *ctx = (TandyContext *)dev->priv;
    if (ctx->count >= TANDY_BUF_SIZE) return -1;
    ctx->buffer[ctx->head] = (char)c;
    ctx->head = (ctx->head + 1) % TANDY_BUF_SIZE;
    ctx->count++;
    return c;
}

static int tandy_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    int written = 0;
    while (*s) {
        if (tandy_putc(dev, (int)(unsigned char)*s) == -1) break;
        s++;
        written++;
    }
    return written;
}

static int tandy_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    TandyContext *ctx = (TandyContext *)dev->priv;
    if (ctx->count <= 0) return -1;
    int c = (unsigned char)ctx->buffer[ctx->tail];
    ctx->tail = (ctx->tail + 1) % TANDY_BUF_SIZE;
    ctx->count--;
    return c;
}

static char *tandy_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size == 0) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int ch = tandy_getc(dev);
        if (ch == -1) break;
        buf[idx++] = (char)ch;
        if (ch == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int tandy_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int tandy_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    TandyContext *ctx = (TandyContext *)dev->priv;
    ctx->head = 0;
    ctx->tail = 0;
    ctx->count = 0;
    return 0;
}

static int tandy_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    TandyContext *ctx = (TandyContext *)d->priv;
    switch (cmd) {
        case 0: // VDIO_RESET
            ctx->head = 0;
            ctx->tail = 0;
            ctx->count = 0;
            return 0;
        case 1: // VDIO_GET_STATUS
            if (arg) *(int *)arg = ctx->online ? 1 : 0;
            return ctx->online ? 1 : 0;
        case 2: // VDIO_SET_BAUD
            if (arg) ctx->baud = *(int *)arg;
            return ctx->baud;
        case 13: // STATUS
            return ctx->online ? 1 : 0;
        default:
            return 0;
    }
}

static int tandy_dev_status(VDev *d) {
    if (!d || !d->priv) return 0;
    TandyContext *ctx = (TandyContext *)d->priv;
    return ctx->online ? 1 : 0;
}

static int tandy_dev_poll(VDev *d) {
    if (!d || !d->priv) return 0;
    TandyContext *ctx = (TandyContext *)d->priv;
    return (ctx->count > 0) ? 1 : 0;
}

static const char *tandy_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    TandyContext *ctx = (TandyContext *)d->priv;
    if (runtime_strcasecmp(key, "CLASS") == 0) return vdev_class_name(ctx->dev_class);
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_tandy";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "RW,STREAM,TANDY_PERIPHERAL";
    return NULL;
}

static int tandy_dev_open(VDev *dev, const char *path, int mode) {
    (void)path;
    if (!dev || !dev->priv) return -1;
    TandyContext *ctx = (TandyContext *)dev->priv;
    if (mode == 1) { // FILE_MODE_OUTPUT
        ctx->head = 0;
        ctx->tail = 0;
        ctx->count = 0;
    }
    return 0;
}

VDev dev_tandy_create(const char *name) {
    TandyContext *ctx = tandy_find_or_create(name);

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = ctx->name;
    dev.dev_class = ctx->dev_class;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "TRS-80 / Tandy Peripheral Driver";
    dev.priv = ctx;

    dev.ops.putc = tandy_putc;
    dev.ops.puts = tandy_puts;
    dev.ops.getc = tandy_getc;
    dev.ops.gets = tandy_gets;
    dev.ops.flush = tandy_flush;
    dev.ops.cls = tandy_cls;

    dev.dev_read = NULL;
    dev.dev_write = NULL;
    dev.dev_seek = NULL;
    dev.dev_ioctl = tandy_dev_ioctl;
    dev.dev_status = tandy_dev_status;
    dev.dev_poll = tandy_dev_poll;
    dev.dev_info = tandy_dev_info;
    dev.dev_open = tandy_dev_open;
    dev.dev_close = NULL;

    return dev;
}
