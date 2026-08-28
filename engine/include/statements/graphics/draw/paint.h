// FILENAME: paint.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (paint.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the PAINT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_PAINT_H
#define STATEMENTS_GRAPHICS_PAINT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_paint_handler(VMContext *vm, LexerContext *lex);

void stmt_paint_register(void);

#endif // STATEMENTS_GRAPHICS_PAINT_H
