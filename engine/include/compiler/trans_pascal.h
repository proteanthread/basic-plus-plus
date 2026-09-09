// FILENAME: trans_pascal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (trans.c, bppc_transpile.c)
// NEEDS: libcore (bppc_internal.h)
// Provides interface definitions for the Object Pascal transpiler backend in BASIC++.
//
// ---- Includes ----

#ifndef COMPILER_TRANS_PASCAL_H
#define COMPILER_TRANS_PASCAL_H

#include <stdbool.h>
#include <stdio.h>

//
// ---- Object Pascal Transpiler Interface ----

bool trans_emit_pascal(void *out, const char *source, bool optimize, bool debug);

#endif // COMPILER_TRANS_PASCAL_H
