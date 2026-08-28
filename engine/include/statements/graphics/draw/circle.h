// FILENAME: circle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (circle.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CIRCLE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_CIRCLE_H
#define STATEMENTS_GRAPHICS_CIRCLE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_circle_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_GRAPHICS_CIRCLE_H
