// FILENAME: mat_input.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_input.c, mat_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MAT_INPUT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_MATRICES_MAT_INPUT_H
#define STATEMENTS_MATRICES_MAT_INPUT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mat_input_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_MATRICES_MAT_INPUT_H
