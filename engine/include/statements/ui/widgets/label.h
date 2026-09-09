// FILENAME: label.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (label.c)
// NEEDS: libcore (types.h)
// Declares the LABEL UI control statement handler interface in BASIC++.

#ifndef LABEL_H
#define LABEL_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_label_handler(VMContext *vm, LexerContext *lex);
void stmt_label_register(void);

#endif // LABEL_H
