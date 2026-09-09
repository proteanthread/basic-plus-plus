// FILENAME: who.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (who.c)
// NEEDS: libcore (types.h)
// Declares the WHO statement handler interface in BASIC++.

#ifndef WHO_H
#define WHO_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_who_handler(VMContext *vm, LexerContext *lex);
void stmt_who_register(void);

#endif // WHO_H
