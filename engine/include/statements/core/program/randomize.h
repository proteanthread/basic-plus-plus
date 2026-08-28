// FILENAME: randomize.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (randomize.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the RANDOMIZE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H
#define STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex);
void stmt_randomize_register(void);

#endif // STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H
