// FILENAME: array_ext.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the ARRAY_EXT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H
#define STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_array_sort_handler(VMContext *vm, LexerContext *lex);
void stmt_arrayext_register(void);

void stmt_array_ext_register(void);

#endif // STATEMENTS_STMT_EXTENDED_STMT_ARRAYEXT_H
