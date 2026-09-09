// FILENAME: dev_atari.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c), libkernel (vdev.c)
// NEEDS: libkernel (dev_atari.h, vdev.h), libhal (hal.h)
// Atari 8-bit Central Input/Output (CIO) device drivers and XIO handler.
//
// ---- Includes ----

#include "device/dev_atari.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

typedef struct {
    char type_char; // 'E', 'K', 'S', 'P', 'C', 'R', 'V', 'D'
    int  unit_num;  // 1..8
    uint8_t cas_block[128]; // 128-byte FSK cassette block buffer
    int  cas_pos;
    bool is_open;
    int  aux1;
    int  aux2;
} AtariDevContext;

static int atari_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    AtariDevContext *ctx = (AtariDevContext *)dev->priv;
    HalContext *hal = hal_get();

    switch (ctx->type_char) {
        case 'E': // Screen Editor
        case 'S': // Screen Display
            if (hal && hal->io.console_putchar) return hal->io.console_putchar(c);
            break;
        case 'P': // Printer
            if (hal && hal->io.console_putchar) return hal->io.console_putchar(c);
            break;
        case 'C': // Cassette (128-byte block accumulator)
            if (ctx->cas_pos < 128) {
                ctx->cas_block[ctx->cas_pos++] = (uint8_t)c;
            }
            if (ctx->cas_pos >= 128) {
                ctx->cas_pos = 0; // block written
            }
            return c;
        default:
            if (hal && hal->io.console_putchar) return hal->io.console_putchar(c);
            break;
    }
    return c;
}

static int atari_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    while (*s) {
        if (atari_putc(dev, (unsigned char)*s) < 0) return -1;
        s++;
    }
    return 0;
}

static int atari_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    AtariDevContext *ctx = (AtariDevContext *)dev->priv;
    HalContext *hal = hal_get();

    switch (ctx->type_char) {
        case 'K': // Raw Keyboard
        case 'E': // Screen Editor
            if (hal && hal->io.console_getchar) return hal->io.console_getchar();
            break;
        case 'C': // Cassette
            if (ctx->cas_pos < 128) {
                return (int)ctx->cas_block[ctx->cas_pos++];
            }
            return -1; // EOF
        default:
            if (hal && hal->io.console_getchar) return hal->io.console_getchar();
            break;
    }
    return -1;
}

static char *atari_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size <= 1) return NULL;
    size_t idx = 0;
    while (idx + 1 < size) {
        int c = atari_getc(dev);
        if (c < 0) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int atari_flush(VDev *dev) {
    HalContext *hal = hal_get();
    if (hal && hal->io.console_flush) hal->io.console_flush();
    return 0;
}

static int atari_cls(VDev *dev) {
    (void)dev;
    HalContext *hal = hal_get();
    if (hal && hal->io.console_puts) hal->io.console_puts("\033[2J\033[H");
    return 0;
}

static int atari_dev_open(VDev *dev, const char *path, int mode) {
    (void)path; (void)mode;
    if (!dev || !dev->priv) return -1;
    AtariDevContext *ctx = (AtariDevContext *)dev->priv;
    ctx->is_open = true;
    ctx->cas_pos = 0;
    return 0;
}

static int atari_dev_close(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    AtariDevContext *ctx = (AtariDevContext *)dev->priv;
    ctx->is_open = false;
    ctx->cas_pos = 0;
    return 0;
}

static int atari_dev_ioctl(VDev *dev, int cmd, void *arg) {
    if (!dev || !dev->priv) return -1;
    AtariDevContext *ctx = (AtariDevContext *)dev->priv;
    switch (cmd) {
        case 3:  // XIO Open/Clear Screen
            atari_cls(dev);
            return 0;
        case 18: // XIO Drawto
            return 0;
        case 32: // XIO Rename
        case 33: // XIO Delete
        case 35: // XIO Lock
        case 36: // XIO Unlock
        case 254:// XIO Format
            return 0;
        default:
            break;
    }
    (void)ctx; (void)arg;
    return 0;
}

int dev_atari_xio_exec(VDev *dev, int cmd, int aux1, int aux2, const char *filespec) {
    (void)filespec;
    if (!dev) return -1;
    if (dev->dev_ioctl) {
        int aux_arr[2] = { aux1, aux2 };
        return dev->dev_ioctl(dev, cmd, aux_arr);
    }
    return 0;
}

VDev dev_atari_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "E:";
    dev.dev_version = "6.5.2";
    dev.dev_caps = VDCAP_STREAM | VDCAP_CONTROL;

    char tc = 'E';
    if (name && *name) {
        tc = (char)runtime_toupper((unsigned char)*name);
    }

    switch (tc) {
        case 'E':
            dev.dev_class = VDCLASS_CONSOLE;
            dev.dev_description = "Atari Screen Editor (CIO Device E:)";
            break;
        case 'K':
            dev.dev_class = VDCLASS_HID;
            dev.dev_description = "Atari Raw Keyboard (CIO Device K:)";
            break;
        case 'S':
            dev.dev_class = VDCLASS_DISPLAY;
            dev.dev_description = "Atari Screen Graphics Surface (CIO Device S:)";
            break;
        case 'P':
            dev.dev_class = VDCLASS_PRINTER;
            dev.dev_description = "Atari Line Printer (CIO Device P:)";
            break;
        case 'C':
            dev.dev_class = VDCLASS_STORAGE;
            dev.dev_description = "Atari Program Recorder 128-Byte FSK Cassette (CIO Device C:)";
            break;
        case 'R':
            dev.dev_class = VDCLASS_SERIAL;
            dev.dev_description = "Atari 850 RS-232 Serial Interface (CIO Device R:)";
            break;
        case 'V':
            dev.dev_class = VDCLASS_CUSTOM;
            dev.dev_description = "Atari Voice Synthesizer (CIO Device V:)";
            break;
        default:
            dev.dev_class = VDCLASS_CUSTOM;
            dev.dev_description = "Atari CIO Virtual Device";
            break;
    }

    dev.ops.putc = atari_putc;
    dev.ops.puts = atari_puts;
    dev.ops.getc = atari_getc;
    dev.ops.gets = atari_gets;
    dev.ops.flush = atari_flush;
    dev.ops.cls = atari_cls;

    dev.dev_open = atari_dev_open;
    dev.dev_close = atari_dev_close;
    dev.dev_ioctl = atari_dev_ioctl;

    static AtariDevContext s_atari_pool[8];
    static size_t s_atari_pool_count = 0;
    AtariDevContext *ctx = NULL;
    if (s_atari_pool_count < 8) {
        ctx = &s_atari_pool[s_atari_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            ctx = (AtariDevContext *)hal->mem.alloc(sizeof(AtariDevContext));
        }
    }
    if (ctx) {
        runtime_memset(ctx, 0, sizeof(AtariDevContext));
        ctx->type_char = tc;
        ctx->unit_num = 1;
    }
    dev.priv = ctx;
    return dev;
}
