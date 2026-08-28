// FILENAME: view.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (view.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VIEW statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_VIEW_H
#define STATEMENTS_GRAPHICS_VIEW_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_view_handler(VMContext *vm, LexerContext *lex);

void stmt_view_register(void);

#endif // STATEMENTS_GRAPHICS_VIEW_H
