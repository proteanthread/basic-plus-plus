// FILENAME: listbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (listbox.c)
// NEEDS: libcore (types.h)
// Declares the LISTBOX UI widget statement handler interface in BASIC++.

#ifndef LISTBOX_H
#define LISTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_listbox_handler(VMContext *vm, LexerContext *lex);
void stmt_listbox_register(void);

#endif // LISTBOX_H
