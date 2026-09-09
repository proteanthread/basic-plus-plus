// FILENAME: dirlistbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dirlistbox.c)
// NEEDS: libcore (types.h)
// Declares the DIRLISTBOX UI control statement handler interface in BASIC++.

#ifndef DIRLISTBOX_H
#define DIRLISTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_dirlistbox_handler(VMContext *vm, LexerContext *lex);
void stmt_dirlistbox_register(void);

#endif // DIRLISTBOX_H
