// FILENAME: void.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (void.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the VOID statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_VOID_H
#define STMT_VOID_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_void_register(void);
BppError stmt_void_handler(VMContext *vm, LexerContext *lex);

#endif // STMT_VOID_H
