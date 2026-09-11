// FILENAME: bppc_freestanding.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bppc_transpile.c, bppc_stmt.c, bppc_freestanding.c)
// NEEDS: libcore (bppc_internal.h)
// Provides freestanding and bare-metal UEFI code generation interfaces for bppc.
//
// ---- Includes ----

#ifndef TOOLS_BPPC_FREESTANDING_H
#define TOOLS_BPPC_FREESTANDING_H

#include <stdbool.h>
#include <stddef.h>

//
// ---- Freestanding & UEFI Code Generation Prototypes ----

void bppc_emit_freestanding_headers(void *out, bool is_uefi);
void bppc_emit_freestanding_entry(void *out, bool is_uefi);
void bppc_emit_freestanding_exit(void *out, bool is_uefi);
void bppc_emit_freestanding_print(void *out, const char *args, bool is_uefi);
bool bppc_is_freestanding_or_uefi(void);

#endif // TOOLS_BPPC_FREESTANDING_H
