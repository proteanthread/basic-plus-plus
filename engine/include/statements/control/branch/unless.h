// FILENAME: unless.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (unless.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the UNLESS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_UNLESS_H
#define STATEMENTS_CONTROL_UNLESS_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_unless_register(void);
BppError stmt_unless_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_CONTROL_UNLESS_H
