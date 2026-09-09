// FILENAME: hscrollbar.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (hscrollbar.c)
// NEEDS: libcore (types.h)
// Declares the HSCROLLBAR UI control statement handler interface in BASIC++.

#ifndef HSCROLLBAR_H
#define HSCROLLBAR_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_hscrollbar_handler(VMContext *vm, LexerContext *lex);
void stmt_hscrollbar_register(void);

#endif // HSCROLLBAR_H
