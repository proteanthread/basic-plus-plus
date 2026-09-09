// FILENAME: dev_pseudo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c), libkernel (vdev.c)
// NEEDS: libkernel (dev_pseudo.h, vdev.h), libhal (hal.h)
// Unix/POSIX and hardware pseudo-device drivers (NULL, ZERO, RANDOM, URANDOM, PORT).
//
// ---- Includes ----

#include "device/dev_pseudo.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

typedef enum {
    PSEUDO_NULL,
    PSEUDO_ZERO,
    PSEUDO_RANDOM,
    PSEUDO_PORT
} PseudoDevType;

typedef struct {
    PseudoDevType type;
    int port_addr;
} PseudoDevContext;

static int pseudo_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    (void)c;
    return c;
}

static int pseudo_puts(VDev *dev, const char *s) {
    (void)dev; (void)s;
    return 0;
}

static int pseudo_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    PseudoDevContext *ctx = (PseudoDevContext *)dev->priv;
    switch (ctx->type) {
        case PSEUDO_NULL:
            return -1; // Immediate EOF
        case PSEUDO_ZERO:
            return 0;  // Infinite null bytes
        case PSEUDO_RANDOM: {
            static uint32_t s_prng = 0x12345678;
            s_prng = s_prng * 1103515245U + 12345U;
            return (int)((s_prng >> 16) & 0xFF);
        }
        case PSEUDO_PORT:
            return 0;
    }
    return -1;
}

static char *pseudo_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size <= 1) return NULL;
    PseudoDevContext *ctx = (PseudoDevContext *)dev->priv;
    if (ctx->type == PSEUDO_NULL) return NULL;
    size_t idx = 0;
    while (idx + 1 < size) {
        int c = pseudo_getc(dev);
        if (c < 0) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int pseudo_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int pseudo_cls(VDev *dev) {
    (void)dev;
    return 0;
}

static int pseudo_dev_read(VDev *dev, void *buf, int len) {
    if (!dev || !dev->priv || !buf || len <= 0) return 0;
    PseudoDevContext *ctx = (PseudoDevContext *)dev->priv;
    uint8_t *dst = (uint8_t *)buf;

    switch (ctx->type) {
        case PSEUDO_NULL:
            return 0; // 0 bytes read = EOF
        case PSEUDO_ZERO:
            runtime_memset(dst, 0, (size_t)len);
            return len;
        case PSEUDO_RANDOM:
            for (int i = 0; i < len; ++i) dst[i] = (uint8_t)(pseudo_getc(dev) & 0xFF);
            return len;
        case PSEUDO_PORT:
            return 0;
    }
    return 0;
}

static int pseudo_dev_write(VDev *dev, const void *buf, int len) {
    (void)dev; (void)buf;
    return len; // All bytes discarded successfully
}

static int pseudo_dev_open(VDev *dev, const char *path, int mode) {
    (void)dev; (void)path; (void)mode;
    return 0;
}

static int pseudo_dev_close(VDev *dev) {
    (void)dev;
    return 0;
}

VDev dev_pseudo_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "NULL:";
    dev.dev_version = "6.5.2";
    dev.dev_caps = VDCAP_STREAM | VDCAP_BINARY;

    PseudoDevType ptype = PSEUDO_NULL;
    if (name) {
        if (runtime_strncasecmp(name, "ZERO", 4) == 0) {
            ptype = PSEUDO_ZERO;
            dev.dev_class = VDCLASS_STORAGE;
            dev.dev_description = "Infinite Null Byte Generator (/dev/zero)";
        } else if (runtime_strncasecmp(name, "RAND", 4) == 0 || runtime_strncasecmp(name, "URAND", 5) == 0) {
            ptype = PSEUDO_RANDOM;
            dev.dev_class = VDCLASS_STORAGE;
            dev.dev_description = "Cryptographic PRNG Random Stream (/dev/urandom)";
        } else if (runtime_strncasecmp(name, "PORT", 4) == 0) {
            ptype = PSEUDO_PORT;
            dev.dev_class = VDCLASS_CUSTOM;
            dev.dev_description = "Hardware CPU I/O Port Bus (8086/Z80 INP/OUT)";
        } else {
            ptype = PSEUDO_NULL;
            dev.dev_class = VDCLASS_STORAGE;
            dev.dev_description = "Bit Bucket Null Sink (/dev/null)";
        }
    }

    dev.ops.putc = pseudo_putc;
    dev.ops.puts = pseudo_puts;
    dev.ops.getc = pseudo_getc;
    dev.ops.gets = pseudo_gets;
    dev.ops.flush = pseudo_flush;
    dev.ops.cls = pseudo_cls;

    dev.dev_read = pseudo_dev_read;
    dev.dev_write = pseudo_dev_write;
    dev.dev_open = pseudo_dev_open;
    dev.dev_close = pseudo_dev_close;

    static PseudoDevContext s_pseudo_pool[8];
    static size_t s_pseudo_pool_count = 0;
    PseudoDevContext *ctx = NULL;
    if (s_pseudo_pool_count < 8) {
        ctx = &s_pseudo_pool[s_pseudo_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            ctx = (PseudoDevContext *)hal->mem.alloc(sizeof(PseudoDevContext));
        }
    }
    if (ctx) {
        runtime_memset(ctx, 0, sizeof(PseudoDevContext));
        ctx->type = ptype;
    }
    dev.priv = ctx;
    return dev;
}
