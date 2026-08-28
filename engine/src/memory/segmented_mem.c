// FILENAME: segmented_mem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c)
// NEEDED BY: libengine (context.c, control.c, data.c, defseg.c)
// NEEDED BY: libengine (dispatch_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, peek.c, poke.c, vm_internal.h)
// NEEDS: libcore (hal.h, memops.h, memops.c, segmented_mem.h)
// NEEDS: libcore (strings.h, strings.c)
// Provides core logic and interface definitions for segmented_mem within BASIC++.
//
// ---- Includes ----

#include "memory/segmented_mem.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
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
    HalContext *hal = hal_get();
    VMemContext *ctx = (VMemContext *)(hal && hal->mem.alloc ? hal->mem.alloc(sizeof(VMemContext)) : NULL);
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(VMemContext));
    ctx->var = var;
    ctx->def_seg = 0x0000;
    
    ctx->handle_capacity = 1024;
    size_t handles_bytes = ctx->handle_capacity * sizeof(HandleEntry);
    ctx->handles = (HandleEntry *)(hal && hal->mem.alloc ? hal->mem.alloc(handles_bytes) : NULL);
    if (!ctx->handles) {
        if (hal && hal->mem.free) hal->mem.free(ctx);
        return NULL;
    }
    runtime_memset(ctx->handles, 0, handles_bytes);
    ctx->handle_count = 0;
    
    // We start handles at 0x10000000 to avoid conflicts with 0x0 segments
    ctx->next_handle = 0x10000000;
    
    return ctx;
}

void vmem_shutdown(VMemContext *ctx) {
    if (ctx) {
        HalContext *hal = hal_get();
        if (ctx->handles && hal && hal->mem.free) hal->mem.free(ctx->handles);
        if (hal && hal->mem.free) hal->mem.free(ctx);
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
    
    // Check if already registered
    for (size_t i = 0; i < ctx->handle_count; i++) {
        if (ctx->handles[i].val == val && ctx->handles[i].is_string_data == is_string_data) {
            return ctx->handles[i].handle;
        }
    }
    
    if (ctx->handle_count >= ctx->handle_capacity) {
        HalContext *hal = hal_get();
        size_t new_cap = ctx->handle_capacity * 2;
        size_t new_bytes = new_cap * sizeof(HandleEntry);
        HandleEntry *new_handles = (HandleEntry *)(hal && hal->mem.alloc ? hal->mem.alloc(new_bytes) : NULL);
        if (!new_handles) return 0; // OOM
        runtime_memset(new_handles, 0, new_bytes);
        runtime_memcpy(new_handles, ctx->handles, ctx->handle_count * sizeof(HandleEntry));
        if (hal && hal->mem.free) hal->mem.free(ctx->handles);
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
        // Not a registered handle. Could be mock BIOS or invalid memory
        return 0;
    }
    
    if (is_string_data && val->type == VAL_STRING) {
        if (val->as.string && str_len(val->as.string) > 0) {
            *out_val = (uint8_t)str_data(val->as.string)[0];
        } else {
            *out_val = 0;
        }
    } else if (val->type == VAL_NUMBER) {
        // Read first byte of a numeric value
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
        // Unmapped memory block. Could be mock BIOS (not handled here) or invalid.
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
