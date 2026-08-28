// FILENAME: incr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (incr.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the INCR statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_INCR_H
#define STMT_INCR_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_incr_register(void);
BppError stmt_incr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_decr_handler(VMContext *vm, LexerContext *lex);

#endif // STMT_INCR_H
