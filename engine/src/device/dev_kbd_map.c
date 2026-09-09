// FILENAME: dev_kbd_map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_kbd_map.h, vdev.h)
// Implementation for programmable keyboard & controller mapping profiles (KBD1..8:).
//
// ---- Includes ----

#include "device/dev_kbd_map.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "runtime/memory/alloc.h"

#define KBD_MAP_BUF_SIZE 1024

typedef struct {
    int  profile;
    char name[16];
    char buffer[KBD_MAP_BUF_SIZE];
    int  head;
    int  tail;
    int  count;
    char key_table[256];
} KbdMapContext;

static KbdMapContext s_kbd_instances[10];
static bool s_kbd_inited = false;

static void kbd_init_all(void) {
    if (s_kbd_inited) return;
    runtime_memset(s_kbd_instances, 0, sizeof(s_kbd_instances));
    for (int i = 0; i < 10; i++) {
        s_kbd_instances[i].profile = i;
        runtime_snprintf(s_kbd_instances[i].name, sizeof(s_kbd_instances[i].name), "KBD%d:", i);
        // Identity mapping by default
        for (int k = 0; k < 256; k++) {
            s_kbd_instances[i].key_table[k] = (char)k;
        }
    }
    s_kbd_inited = true;
}

static int kbd_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    KbdMapContext *ctx = (KbdMapContext *)dev->priv;
    if (ctx->count >= KBD_MAP_BUF_SIZE) return -1;
    char translated = ctx->key_table[(unsigned char)c];
    ctx->buffer[ctx->head] = translated;
    ctx->head = (ctx->head + 1) % KBD_MAP_BUF_SIZE;
    ctx->count++;
    return (unsigned char)translated;
}

static int kbd_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    int written = 0;
    while (*s) {
        if (kbd_putc(dev, (int)(unsigned char)*s) == -1) break;
        s++;
        written++;
    }
    return written;
}

static int kbd_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    KbdMapContext *ctx = (KbdMapContext *)dev->priv;
    if (ctx->count <= 0) return -1;
    int c = (unsigned char)ctx->buffer[ctx->tail];
    ctx->tail = (ctx->tail + 1) % KBD_MAP_BUF_SIZE;
    ctx->count--;
    return c;
}

static char *kbd_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size == 0) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int ch = kbd_getc(dev);
        if (ch == -1) break;
        buf[idx++] = (char)ch;
        if (ch == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int kbd_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int kbd_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    KbdMapContext *ctx = (KbdMapContext *)dev->priv;
    ctx->head = 0;
    ctx->tail = 0;
    ctx->count = 0;
    return 0;
}

static int kbd_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    KbdMapContext *ctx = (KbdMapContext *)d->priv;
    switch (cmd) {
        case 0: // RESET
            ctx->head = 0;
            ctx->tail = 0;
            ctx->count = 0;
            return 0;
        case 13: // STATUS
            return ctx->count;
        default:
            return 0;
    }
}

static int kbd_dev_status(VDev *d) {
    if (!d || !d->priv) return 0;
    KbdMapContext *ctx = (KbdMapContext *)d->priv;
    return ctx->count;
}

static int kbd_dev_poll(VDev *d) {
    if (!d || !d->priv) return 0;
    KbdMapContext *ctx = (KbdMapContext *)d->priv;
    return (ctx->count > 0) ? 1 : 0;
}

static const char *kbd_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "KEYMAP";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_kbd_map";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "READ,WRITE,KEYMAP,CONTROLLER_MAP";
    return NULL;
}

VDev dev_kbd_map_create(const char *name, int profile) {
    kbd_init_all();
    int pr = (profile >= 0 && profile <= 9) ? profile : 1;
    KbdMapContext *ctx = &s_kbd_instances[pr];
    if (name && name[0]) {
        runtime_snprintf(ctx->name, sizeof(ctx->name), "%s", name);
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = ctx->name;
    dev.dev_class = VDCLASS_KEYMAP;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Programmable Keyboard & Controller Map";
    dev.priv = ctx;

    dev.ops.putc = kbd_putc;
    dev.ops.puts = kbd_puts;
    dev.ops.getc = kbd_getc;
    dev.ops.gets = kbd_gets;
    dev.ops.flush = kbd_flush;
    dev.ops.cls = kbd_cls;

    dev.dev_read = NULL;
    dev.dev_write = NULL;
    dev.dev_seek = NULL;
    dev.dev_ioctl = kbd_dev_ioctl;
    dev.dev_status = kbd_dev_status;
    dev.dev_poll = kbd_dev_poll;
    dev.dev_info = kbd_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
