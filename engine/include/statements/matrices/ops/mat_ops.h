// FILENAME: mat_ops.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MAT_OPS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_MATRICES_MAT_OPS_H
#define STATEMENTS_MATRICES_MAT_OPS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mat_ops_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mat_handler(VMContext *vm, LexerContext *lex);

void stmt_mat_ops_register(void);

#endif // STATEMENTS_MATRICES_MAT_OPS_H
