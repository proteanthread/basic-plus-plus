// FILENAME: decr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (decr.c)
// NEEDS: libcore (types.h)
// Declares the DECR statement handler interface in BASIC++.

#ifndef DECR_H
#define DECR_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_decr_handler(VMContext *vm, LexerContext *lex);
void stmt_decr_register(void);

#endif // DECR_H
