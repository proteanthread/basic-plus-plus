// FILENAME: const.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (const.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the CONST statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_CONST_H
#define STATEMENTS_VARIABLES_CONST_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_const_handler(VMContext *vm, LexerContext *lex);
void stmt_const_register(void);

#endif // STATEMENTS_VARIABLES_CONST_H
