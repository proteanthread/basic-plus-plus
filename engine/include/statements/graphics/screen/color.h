// FILENAME: color.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (color.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the COLOR statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_COLOR_H
#define STATEMENTS_GRAPHICS_COLOR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_color_handler(VMContext *vm, LexerContext *lex);

BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex);

BppError stmt_locate_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_GRAPHICS_COLOR_H
