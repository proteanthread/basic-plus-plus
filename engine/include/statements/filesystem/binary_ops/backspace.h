// FILENAME: backspace.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (backspace.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the BACKSPACE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_BACKSPACE_H
#define STATEMENTS_FILESYSTEM_BACKSPACE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_backspace_register(void);
BppError stmt_backspace_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_BACKSPACE_H
