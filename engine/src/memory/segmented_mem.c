/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file segmented_mem.c
 * @brief Virtual Segment Emulator for BASIC++
 * 
 * What it does: Safe emulation of segmented memory.
 * Why it exists: Prevent raw pointer dereferencing in 64-bit space.
 * Why it works this way: Uses an opaque handle array.
 * What can be changed: Internal handle storage capacity.
 * What cannot be changed: The 16-bit segmented logic behavior.
 * Portability concerns: None.
 */

#include "memory/segmented_mem.h"
#include <stdlib.h>
#include <string.h>
#include "runtime/strings.h"

#define MAX_HANDLES 65536

typedef struct {
    uint32_t handle;
    BValue *val;
    bool is_string_data;
} HandleEntry;

struct VMemContext {
    VariableContext *var;
    uint16_t def_seg;
    
    HandleEntry *handles;
    size_t handle_count;
    size_t handle_capacity;
    uint32_t next_handle;
};

VMemContext *vmem_init(VariableContext *var) {
    VMemContext *ctx = (VMemContext *)calloc(1, sizeof(VMemContext));
    if (!ctx) return NULL;
    ctx->var = var;
    ctx->def_seg = 0x0000;
    
    ctx->handle_capacity = 1024;
    ctx->handles = (HandleEntry *)calloc(ctx->handle_capacity, sizeof(HandleEntry));
    if (!ctx->handles) {
        free(ctx);
        return NULL;
    }
    ctx->handle_count = 0;
    
    /* We start handles at 0x10000000 to avoid conflicts with 0x0 segments */
    ctx->next_handle = 0x10000000;
    
    return ctx;
}

void vmem_shutdown(VMemContext *ctx) {
    if (ctx) {
        if (ctx->handles) free(ctx->handles);
        free(ctx);
    }
}

void vmem_set_def_seg(VMemContext *ctx, uint16_t seg) {
    if (ctx) ctx->def_seg = seg;
}

uint16_t vmem_get_def_seg(VMemContext *ctx) {
    return ctx ? ctx->def_seg : 0;
}

uint32_t vmem_register_handle(VMemContext *ctx, BValue *val, bool is_string_data) {
    if (!ctx || !val) return 0;
    
    /* Check if already registered */
    for (size_t i = 0; i < ctx->handle_count; i++) {
        if (ctx->handles[i].val == val && ctx->handles[i].is_string_data == is_string_data) {
            return ctx->handles[i].handle;
        }
    }
    
    if (ctx->handle_count >= ctx->handle_capacity) {
        size_t new_cap = ctx->handle_capacity * 2;
        HandleEntry *new_handles = (HandleEntry *)realloc(ctx->handles, new_cap * sizeof(HandleEntry));
        if (!new_handles) return 0; /* OOM */
        memset(new_handles + ctx->handle_capacity, 0, (new_cap - ctx->handle_capacity) * sizeof(HandleEntry));
        ctx->handles = new_handles;
        ctx->handle_capacity = new_cap;
    }
    
    uint32_t handle = ctx->next_handle++;
    ctx->handles[ctx->handle_count].handle = handle;
    ctx->handles[ctx->handle_count].val = val;
    ctx->handles[ctx->handle_count].is_string_data = is_string_data;
    ctx->handle_count++;
    
    return handle;
}

BValue *vmem_resolve_handle(VMemContext *ctx, uint32_t handle, bool *is_string_data) {
    if (!ctx) return NULL;
    for (size_t i = 0; i < ctx->handle_count; i++) {
        if (ctx->handles[i].handle == handle) {
            if (is_string_data) *is_string_data = ctx->handles[i].is_string_data;
            return ctx->handles[i].val;
        }
    }
    return NULL;
}

int vmem_peek(VMemContext *ctx, uint16_t address, uint8_t *out_val) {
    if (!ctx || !out_val) return -1;
    
    uint32_t handle = ((uint32_t)ctx->def_seg << 16) | address;
    bool is_string_data = false;
    BValue *val = vmem_resolve_handle(ctx, handle, &is_string_data);
    
    if (!val) {
        /* Not a registered handle. Could be mock BIOS or invalid memory */
        return 0;
    }
    
    if (is_string_data && val->type == VAL_STRING) {
        if (val->as.string && str_len(val->as.string) > 0) {
            *out_val = (uint8_t)str_data(val->as.string)[0];
        } else {
            *out_val = 0;
        }
    } else if (val->type == VAL_NUMBER) {
        /* Read first byte of a numeric value */
        double d = val->as.number;
        uint8_t *p = (uint8_t *)&d;
        *out_val = p[0];
    } else {
        *out_val = 0;
    }
    
    return 1;
}

int vmem_poke(VMemContext *ctx, uint16_t address, uint8_t val_to_write) {
    if (!ctx) return -1;
    
    uint32_t handle = ((uint32_t)ctx->def_seg << 16) | address;
    bool is_string_data = false;
    BValue *val = vmem_resolve_handle(ctx, handle, &is_string_data);
    
    if (!val) {
        /* Unmapped memory block. Could be mock BIOS (not handled here) or invalid. */
        return 0;
    }
    
    if (is_string_data && val->type == VAL_STRING) {
        if (val->as.string && str_len(val->as.string) > 0) {
            char *ptr = (char *)str_data(val->as.string);
            ptr[0] = (char)val_to_write;
        }
    } else if (val->type == VAL_NUMBER) {
        double d = val->as.number;
        uint8_t *p = (uint8_t *)&d;
        p[0] = val_to_write;
        val->as.number = d;
    }
    
    return 1;
}
