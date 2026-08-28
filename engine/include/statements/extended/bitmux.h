// FILENAME: bitmux.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the BITMUX statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_BITMUX_H
#define STATEMENTS_STMT_BITMUX_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);
void stmt_bitmux_register(void);

#endif // STATEMENTS_STMT_BITMUX_H
