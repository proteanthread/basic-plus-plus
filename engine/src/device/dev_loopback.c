// FILENAME: dev_loopback.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_loopback.h, vdev.h), libplatform (plat_mem.h)
// Implementation for full-duplex FIFO loopback testing device (LOOP1..8:, LOOP0:, LOOP9:).
//
// ---- Includes ----

#include "device/dev_loopback.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "runtime/memory/alloc.h"

#define LOOPBACK_BUF_SIZE 4096

typedef struct {
    int  channel;
    char buffer[LOOPBACK_BUF_SIZE];
    int  head;
    int  tail;
    int  count;
    char name_buf[16];
} LoopbackContext;

static LoopbackContext s_loop_instances[10];
static bool s_loop_inited = false;

static void loopback_init_all(void) {
    if (s_loop_inited) return;
    runtime_memset(s_loop_instances, 0, sizeof(s_loop_instances));
    for (int i = 0; i < 10; i++) {
        s_loop_instances[i].channel = i;
        runtime_snprintf(s_loop_instances[i].name_buf, sizeof(s_loop_instances[i].name_buf), "LOOP%d:", i);
    }
    s_loop_inited = true;
}

static int loopback_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    LoopbackContext *ctx = (LoopbackContext *)dev->priv;
    if (ctx->count >= LOOPBACK_BUF_SIZE) return -1; // Overflow
    ctx->buffer[ctx->head] = (char)c;
    ctx->head = (ctx->head + 1) % LOOPBACK_BUF_SIZE;
    ctx->count++;
    return c;
}

static int loopback_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    int written = 0;
    while (*s) {
        if (loopback_putc(dev, (int)(unsigned char)*s) == -1) break;
        s++;
        written++;
    }
    return written;
}

static int loopback_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    LoopbackContext *ctx = (LoopbackContext *)dev->priv;
    if (ctx->count <= 0) return -1; // Underflow
    int c = (unsigned char)ctx->buffer[ctx->tail];
    ctx->tail = (ctx->tail + 1) % LOOPBACK_BUF_SIZE;
    ctx->count--;
    return c;
}

static char *loopback_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size == 0) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int ch = loopback_getc(dev);
        if (ch == -1) break;
        buf[idx++] = (char)ch;
        if (ch == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int loopback_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int loopback_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    LoopbackContext *ctx = (LoopbackContext *)dev->priv;
    ctx->head = 0;
    ctx->tail = 0;
    ctx->count = 0;
    return 0;
}

static int loopback_dev_read(VDev *d, void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char *out = (char *)buf;
    int read_bytes = 0;
    for (int i = 0; i < len; i++) {
        int ch = loopback_getc(d);
        if (ch == -1) break;
        out[i] = (char)ch;
        read_bytes++;
    }
    return read_bytes;
}

static int loopback_dev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    const char *in = (const char *)buf;
    int written_bytes = 0;
    for (int i = 0; i < len; i++) {
        if (loopback_putc(d, (int)(unsigned char)in[i]) == -1) break;
        written_bytes++;
    }
    return written_bytes;
}

static int loopback_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    LoopbackContext *ctx = (LoopbackContext *)d->priv;
    switch (cmd) {
        case 0: // VDIO_RESET / Clear
            ctx->head = 0;
            ctx->tail = 0;
            ctx->count = 0;
            return 0;
        case 1: // VDIO_GET_STATUS
            if (arg) *(int *)arg = ctx->count;
            return ctx->count;
        case 13: // Atari STATUS
            return ctx->count;
        case 254: // IOCB RESET
            ctx->head = 0;
            ctx->tail = 0;
            ctx->count = 0;
            return 0;
        default:
            return 0;
    }
}

static int loopback_dev_status(VDev *d) {
    if (!d || !d->priv) return 0;
    LoopbackContext *ctx = (LoopbackContext *)d->priv;
    return ctx->count;
}

static int loopback_dev_poll(VDev *d) {
    if (!d || !d->priv) return 0;
    LoopbackContext *ctx = (LoopbackContext *)d->priv;
    return (ctx->count > 0) ? 1 : 0;
}

static const char *loopback_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "LOOPBACK";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_loopback";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "RW,STREAM,DUPLEX,BUFFERED";
    return NULL;
}

static int loopback_dev_open(VDev *dev, const char *path, int mode) {
    (void)path;
    if (!dev || !dev->priv) return -1;
    LoopbackContext *ctx = (LoopbackContext *)dev->priv;
    if (mode == 1) { // FILE_MODE_OUTPUT
        ctx->head = 0;
        ctx->tail = 0;
        ctx->count = 0;
    }
    return 0;
}

VDev dev_loopback_create(const char *name, int channel) {
    loopback_init_all();
    int ch = (channel >= 0 && channel <= 9) ? channel : 1;
    LoopbackContext *ctx = &s_loop_instances[ch];
    if (name && name[0]) {
        runtime_snprintf(ctx->name_buf, sizeof(ctx->name_buf), "%s", name);
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = ctx->name_buf;
    dev.dev_class = VDCLASS_LOOPBACK;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Full-Duplex FIFO Loopback Testing Device";
    dev.priv = ctx;

    dev.ops.putc = loopback_putc;
    dev.ops.puts = loopback_puts;
    dev.ops.getc = loopback_getc;
    dev.ops.gets = loopback_gets;
    dev.ops.flush = loopback_flush;
    dev.ops.cls = loopback_cls;

    dev.dev_read = loopback_dev_read;
    dev.dev_write = loopback_dev_write;
    dev.dev_seek = NULL;
    dev.dev_ioctl = loopback_dev_ioctl;
    dev.dev_status = loopback_dev_status;
    dev.dev_poll = loopback_dev_poll;
    dev.dev_info = loopback_dev_info;
    dev.dev_open = loopback_dev_open;
    dev.dev_close = NULL;

    return dev;
}
