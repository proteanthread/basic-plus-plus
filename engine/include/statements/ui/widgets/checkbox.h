// FILENAME: checkbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (checkbox.c)
// NEEDS: libcore (types.h)
// Declares the CHECKBOX UI control statement handler interface in BASIC++.

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_checkbox_handler(VMContext *vm, LexerContext *lex);
void stmt_checkbox_register(void);

#endif // CHECKBOX_H
