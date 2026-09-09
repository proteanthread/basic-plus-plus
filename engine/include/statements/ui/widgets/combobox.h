// FILENAME: combobox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (combobox.c)
// NEEDS: libcore (types.h)
// Declares the COMBOBOX UI control statement handler interface in BASIC++.

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_combobox_handler(VMContext *vm, LexerContext *lex);
void stmt_combobox_register(void);

#endif // COMBOBOX_H
