// FILENAME: clr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (clr.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the CLR statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_CLR_H
#define STMT_CLR_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_clr_register(void);
BppError stmt_clr_handler(VMContext *vm, LexerContext *lex);

#endif // STMT_CLR_H
