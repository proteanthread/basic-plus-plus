// FILENAME: vscrollbar.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (vscrollbar.c)
// NEEDS: libcore (types.h)
// Declares the VSCROLLBAR UI control statement handler interface in BASIC++.

#ifndef VSCROLLBAR_H
#define VSCROLLBAR_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_vscrollbar_handler(VMContext *vm, LexerContext *lex);
void stmt_vscrollbar_register(void);

#endif // VSCROLLBAR_H
