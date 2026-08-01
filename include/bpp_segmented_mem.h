/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_segmented_mem.h
 * @brief Virtual Segment Emulator for BASIC++
 * 
 * What it does: Provides safe emulation of legacy 16-bit segmented memory (DEF SEG, VARPTR, PEEK, POKE).
 * Why it exists: To allow legacy memory manipulation scripts to function in a modern 64-bit protected environment without returning raw OS pointers.
 * Why it works this way: It generates 32-bit virtual handles for variables. VARSEG returns the upper 16 bits, VARPTR returns the lower 16 bits. PEEK and POKE combine DEF SEG (upper 16) and address (lower 16) to reconstruct the handle and resolve the variable safely.
 * What can be changed: Internal handle lookup mechanisms (hash table vs array).
 * What cannot be changed: The 16-bit bounds of VARPTR and VARSEG.
 * Portability concerns: None, strictly hardware independent.
 */

#ifndef BPP_SEGMENTED_MEM_H
#define BPP_SEGMENTED_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "bpp_types.h"
#include "bpp_variables.h"

typedef struct VMemContext VMemContext;

VMemContext *vmem_init(VariableContext *var);
void vmem_shutdown(VMemContext *ctx);

void vmem_set_def_seg(VMemContext *ctx, uint16_t seg);
uint16_t vmem_get_def_seg(VMemContext *ctx);

/* Generates or retrieves a 32-bit virtual handle for a variable pointer */
uint32_t vmem_register_handle(VMemContext *ctx, BValue *val, bool is_string_data);

/* Resolves a 32-bit handle to its registered BValue * */
BValue *vmem_resolve_handle(VMemContext *ctx, uint32_t handle, bool *is_string_data);

/* Safe memory access functions (address is a 16-bit offset from current DEF SEG) */
int vmem_peek(VMemContext *ctx, uint16_t address, uint8_t *out_val);
int vmem_poke(VMemContext *ctx, uint16_t address, uint8_t val);

#endif /* BPP_SEGMENTED_MEM_H */
