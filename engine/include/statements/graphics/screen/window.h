// FILENAME: window.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (window.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the WINDOW statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_WINDOW_H
#define STATEMENTS_GRAPHICS_WINDOW_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_window_handler(VMContext *vm, LexerContext *lex);

void stmt_window_register(void);

#endif // STATEMENTS_GRAPHICS_WINDOW_H
