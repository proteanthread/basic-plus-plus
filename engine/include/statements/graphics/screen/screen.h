// FILENAME: screen.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (screen.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SCREEN statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_SCREEN_H
#define STATEMENTS_GRAPHICS_SCREEN_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);

BppError stmt_title_handler(VMContext *vm, LexerContext *lex);

BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex);

BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex);

BppError stmt_resize_handler(VMContext *vm, LexerContext *lex);

BppError stmt_icon_handler(VMContext *vm, LexerContext *lex);

BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex);

BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_GRAPHICS_SCREEN_H
