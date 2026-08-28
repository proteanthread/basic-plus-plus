// FILENAME: try.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the TRY statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_EXTENDED_STMT_TRY_H
#define STATEMENTS_STMT_EXTENDED_STMT_TRY_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_try_handler(VMContext *vm, LexerContext *lex);
void stmt_try_register(void);

#endif // STATEMENTS_STMT_EXTENDED_STMT_TRY_H
