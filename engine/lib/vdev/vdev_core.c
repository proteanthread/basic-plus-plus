// FILENAME: vdev_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides core logic and interface definitions for vdev_core within BASIC++.
//
// ---- Includes ----

#include "device/vdev.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define VDEV_MAX_DEVICES 64

struct VDevContext {
    MemoryContext *mem;
    VDev           devices[VDEV_MAX_DEVICES];
    size_t         count;
};

static int strcmp_nocase(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = toupper((unsigned char)*s1);
        int c2 = toupper((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

VDevContext *vdev_init(MemoryContext *mem) {
    VDevContext *ctx = (VDevContext *)calloc(1, sizeof(VDevContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->count = 0;
    return ctx;
}

void vdev_shutdown(VDevContext *ctx) {
    if (!ctx) return;
    for (size_t i = 0; i < ctx->count; i++) {
        if (ctx->devices[i].close) {
            ctx->devices[i].close(ctx->devices[i].user_data);
        }
    }
    free(ctx);
}

bool vdev_register(VDevContext *ctx, VDev dev) {
    if (!ctx || ctx->count >= VDEV_MAX_DEVICES) return false;
    for (size_t i = 0; i < ctx->count; i++) {
        if (strcmp_nocase(ctx->devices[i].name, dev.name) == 0) {
            return false; // Duplicate device name
        }
    }
    ctx->devices[ctx->count++] = dev;
    return true;
}

VDev *vdev_get(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;
    for (size_t i = 0; i < ctx->count; i++) {
        if (strcmp_nocase(ctx->devices[i].name, name) == 0) {
            return &ctx->devices[i];
        }
    }
    return NULL;
}
