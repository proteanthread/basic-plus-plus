// FILENAME: reformat_unpack.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat_unpack.c, reformat.c)
// NEEDS: libengine (vm.h)
// Provides interface for unpacking space-stripped packed vintage BASIC source lines.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_REFORMAT_UNPACK_H
#define STATEMENTS_PROGRAM_REFORMAT_UNPACK_H

#include <stdbool.h>
#include <stddef.h>
#include "vm/vm.h"

// Analyzes whether a line contains packed vintage syntax (e.g. 10FORI=1TO10:GOTO100)
bool reformat_is_packed_line(const char *src_line);

// Unpacks a source line into the destination buffer with full Pascal/Algol spacing.
bool reformat_unpack_buffer(const char *src_line, char *out, size_t out_cap);

// Unpacks a single line by injecting standard spaces around keywords, operators, and colons
// Allocates the returned string using the provided memory context.
char *reformat_unpack_line(const char *src_line, void *mem_ctx);

// Unpacks all lines of the active in-memory program. Returns count of modified lines.
int reformat_unpack_program(VMContext *vm);

// Decompresses vintage short-form OPEN ("mode", [#]ch, "filename" [, reclen]) into canonical OPEN.
bool reformat_expand_open_buffer(const char *src_line, char *out, size_t out_cap);
int reformat_expand_open_program(VMContext *vm);

#endif // STATEMENTS_PROGRAM_REFORMAT_UNPACK_H
