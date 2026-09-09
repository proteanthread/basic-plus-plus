// FILENAME: priority.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (priority.c)
// NEEDS: libcore (types.h)
// Declares the PRIORITY statement handler interface in BASIC++.

#ifndef PRIORITY_H
#define PRIORITY_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_priority_handler(VMContext *vm, LexerContext *lex);
void stmt_priority_register(void);

#endif // PRIORITY_H
