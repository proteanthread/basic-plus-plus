// FILENAME: collate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the COLLATE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EXTENDED_COLLATE_H
#define STATEMENTS_EXTENDED_COLLATE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_collate_handler(VMContext *vm, LexerContext *lex);
void stmt_collate_register(void);

#endif // STATEMENTS_EXTENDED_COLLATE_H
