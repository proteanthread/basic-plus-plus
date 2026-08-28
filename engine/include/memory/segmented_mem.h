// FILENAME: segmented_mem.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c, segmented_mem.c)
// NEEDED BY: libengine (context.c, control.c, data.c, defseg.c)
// NEEDED BY: libengine (dispatch_internal.h, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, peek.c, poke.c, vm_internal.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for segmented_mem within BASIC++.
//
// ---- Includes ----

#ifndef MEMORY_SEGMENTED_MEM_H
#define MEMORY_SEGMENTED_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "types/types.h"
#include "runtime/variables.h"

typedef struct VMemContext VMemContext;

VMemContext *vmem_init(VariableContext *var);
void vmem_shutdown(VMemContext *ctx);

void vmem_set_def_seg(VMemContext *ctx, uint16_t seg);
uint16_t vmem_get_def_seg(VMemContext *ctx);

// Generates or retrieves a 32-bit virtual handle for a variable pointer
uint32_t vmem_register_handle(VMemContext *ctx, BValue *val, bool is_string_data);

// Resolves a 32-bit handle to its registered BValue *
BValue *vmem_resolve_handle(VMemContext *ctx, uint32_t handle, bool *is_string_data);

// Safe memory access functions (address is a 16-bit offset from current DEF SEG)
int vmem_peek(VMemContext *ctx, uint16_t address, uint8_t *out_val);
int vmem_poke(VMemContext *ctx, uint16_t address, uint8_t val);

#endif // MEMORY_SEGMENTED_MEM_H
