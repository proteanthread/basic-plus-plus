// FILENAME: incr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (incr.c)
// NEEDS: libcore (types.h)
// Declares the INCR statement handler interface in BASIC++.

#ifndef INCR_H
#define INCR_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_incr_handler(VMContext *vm, LexerContext *lex);
void stmt_incr_register(void);

#endif // INCR_H
