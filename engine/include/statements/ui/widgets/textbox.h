// FILENAME: textbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (textbox.c)
// NEEDS: libcore (types.h)
// Declares the TEXTBOX UI widget statement handler interface in BASIC++.

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_textbox_handler(VMContext *vm, LexerContext *lex);
void stmt_textbox_register(void);

#endif // TEXTBOX_H
