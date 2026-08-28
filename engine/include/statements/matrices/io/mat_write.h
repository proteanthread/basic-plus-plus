// FILENAME: mat_write.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h, mat_write.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MAT_WRITE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_MATRICES_MAT_WRITE_H
#define STATEMENTS_MATRICES_MAT_WRITE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void     stmt_mat_write_register(void);
BppError stmt_mat_write_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_MATRICES_MAT_WRITE_H
