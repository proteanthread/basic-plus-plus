// FILENAME: dev_cpm.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c), libkernel (vdev.c)
// NEEDS: libkernel (dev_cpm.h, vdev.h), libhal (hal.h)
// CP/M 2.2 and MP/M peripheral virtual device driver (RDR, PUN, LST, AXI, AXO, BAT).
//
// ---- Includes ----

#include "device/dev_cpm.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

typedef struct {
    char type_code[8];
    uint8_t buffer[512];
    int head;
    int tail;
    bool is_open;
    int leader_count;
} CpmDevContext;

static int cpm_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    CpmDevContext *ctx = (CpmDevContext *)dev->priv;
    HalContext *hal = hal_get();

    if (runtime_strcasecmp(ctx->type_code, "PUN") == 0 || runtime_strcasecmp(ctx->type_code, "PTP") == 0) {
        // Paper tape punch: 8-level ASCII
        int next = (ctx->head + 1) % 512;
        if (next != ctx->tail) {
            ctx->buffer[ctx->head] = (uint8_t)c;
            ctx->head = next;
            return c;
        }
        return -1;
    }
    if (runtime_strcasecmp(ctx->type_code, "LST") == 0) {
        // List printer
        if (hal && hal->io.console_putchar) return hal->io.console_putchar(c);
    }
    if (hal && hal->io.console_putchar) return hal->io.console_putchar(c);
    return c;
}

static int cpm_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    while (*s) {
        if (cpm_putc(dev, (unsigned char)*s) < 0) return -1;
        s++;
    }
    return 0;
}

static int cpm_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    CpmDevContext *ctx = (CpmDevContext *)dev->priv;
    HalContext *hal = hal_get();

    if (runtime_strcasecmp(ctx->type_code, "RDR") == 0 || runtime_strcasecmp(ctx->type_code, "PTR") == 0) {
        if (ctx->head != ctx->tail) {
            uint8_t ch = ctx->buffer[ctx->tail];
            ctx->tail = (ctx->tail + 1) % 512;
            return (int)ch;
        }
        return 0x1A; // CP/M EOF (Ctrl-Z)
    }
    if (hal && hal->io.console_getchar) return hal->io.console_getchar();
    return -1;
}

static char *cpm_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size <= 1) return NULL;
    size_t idx = 0;
    while (idx + 1 < size) {
        int c = cpm_getc(dev);
        if (c < 0 || c == 0x1A) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int cpm_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int cpm_cls(VDev *dev) {
    (void)dev;
    return 0;
}

static int cpm_dev_open(VDev *dev, const char *path, int mode) {
    (void)path; (void)mode;
    if (!dev || !dev->priv) return -1;
    CpmDevContext *ctx = (CpmDevContext *)dev->priv;
    ctx->is_open = true;
    ctx->head = 0;
    ctx->tail = 0;
    return 0;
}

static int cpm_dev_close(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    CpmDevContext *ctx = (CpmDevContext *)dev->priv;
    ctx->is_open = false;
    return 0;
}

VDev dev_cpm_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "PUN:";
    dev.dev_version = "6.5.2";
    dev.dev_caps = VDCAP_STREAM | VDCAP_BINARY;

    char base_type[8] = "PUN";
    if (name && *name) {
        size_t i = 0;
        while (name[i] && name[i] != ':' && i + 1 < sizeof(base_type)) {
            base_type[i] = (char)runtime_toupper((unsigned char)name[i]);
            i++;
        }
        base_type[i] = '\0';
    }

    if (runtime_strcasecmp(base_type, "PUN") == 0 || runtime_strcasecmp(base_type, "PTP") == 0) {
        dev.dev_class = VDCLASS_STORAGE;
        dev.dev_description = "CP/M Paper Tape Punch Stream (8-Level Tape)";
    } else if (runtime_strcasecmp(base_type, "RDR") == 0 || runtime_strcasecmp(base_type, "PTR") == 0) {
        dev.dev_class = VDCLASS_STORAGE;
        dev.dev_description = "CP/M Paper Tape Reader Stream (XON/XOFF Controlled)";
    } else if (runtime_strcasecmp(base_type, "LST") == 0) {
        dev.dev_class = VDCLASS_PRINTER;
        dev.dev_description = "CP/M List Printer Stream";
    } else {
        dev.dev_class = VDCLASS_SERIAL;
        dev.dev_description = "CP/M Auxiliary Communications Stream";
    }

    dev.ops.putc = cpm_putc;
    dev.ops.puts = cpm_puts;
    dev.ops.getc = cpm_getc;
    dev.ops.gets = cpm_gets;
    dev.ops.flush = cpm_flush;
    dev.ops.cls = cpm_cls;

    dev.dev_open = cpm_dev_open;
    dev.dev_close = cpm_dev_close;

    static CpmDevContext s_cpm_pool[8];
    static size_t s_cpm_pool_count = 0;
    CpmDevContext *ctx = NULL;
    if (s_cpm_pool_count < 8) {
        ctx = &s_cpm_pool[s_cpm_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            ctx = (CpmDevContext *)hal->mem.alloc(sizeof(CpmDevContext));
        }
    }
    if (ctx) {
        runtime_memset(ctx, 0, sizeof(CpmDevContext));
        runtime_strncpy(ctx->type_code, base_type, sizeof(ctx->type_code) - 1);
        ctx->leader_count = 10;
    }
    dev.priv = ctx;
    return dev;
}
