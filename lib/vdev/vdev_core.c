/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Implements the standalone Virtual Device System core registry and routing.
 * Why it exists: Decouples devices (console, files, graphics, bus) into Ring 2 abstractions.
 * Why it works this way: Maintains an array of VDev slots with string name matching.
 * What can be changed: VDEV_MAX_DEVICES registry limit.
 * What cannot be changed: VDevContext handle lifecycle and device registration contract.
 * What to expect: Fast O(N) device lookups by string identifier (e.g. "CON:", "SCR:").
 * What to do if something breaks: Check VDev name normalization and NULL pointer checks.
 * Assumptions: Device names are 7-bit ASCII strings.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add dynamic device unregistration and event queues.
 * External extension hooks: Exposed via bpp_vdev.h.
 */

#include "bpp_vdev.h"
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
            return false; /* Duplicate device name */
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
