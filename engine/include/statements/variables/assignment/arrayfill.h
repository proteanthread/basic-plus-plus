// FILENAME: arrayfill.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (arrayfill.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the ARRAYFILL statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_ARRAYFILL_H
#define STMT_ARRAYFILL_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_arrayfill_register(void);
BppError stmt_arrayfill_handler(VMContext *vm, LexerContext *lex);

#endif // STMT_ARRAYFILL_H
