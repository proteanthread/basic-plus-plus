// FILENAME: mouse.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mouse.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MOUSE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_MOUSE_H
#define STATEMENTS_SYSTEM_MOUSE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mouse_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex);

BppError stmt_hmouse_handler(VMContext *vm, LexerContext *lex);

BppError stmt_vmouse_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SYSTEM_MOUSE_H
