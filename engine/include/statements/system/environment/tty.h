// FILENAME: tty.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (tty.c)
// NEEDS: libcore (types.h)
// Declares the TTY statement handler interface in BASIC++.

#ifndef TTY_H
#define TTY_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_tty_handler(VMContext *vm, LexerContext *lex);
void stmt_tty_register(void);

#endif // TTY_H
