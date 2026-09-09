// FILENAME: button.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (button.c)
// NEEDS: libcore (types.h)
// Declares the BUTTON UI widget statement handler interface in BASIC++.

#ifndef BUTTON_H
#define BUTTON_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_button_handler(VMContext *vm, LexerContext *lex);
void stmt_button_register(void);

#endif // BUTTON_H
