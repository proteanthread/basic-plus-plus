// FILENAME: rewind.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rewind.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the REWIND statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_REWIND_H
#define STATEMENTS_FILESYSTEM_REWIND_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_rewind_register(void);
BppError stmt_rewind_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_REWIND_H
