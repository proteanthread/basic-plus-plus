// FILENAME: login.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (login.c)
// NEEDS: libcore (types.h)
// Declares the LOGIN / HELLO statement handler interface in BASIC++.

#ifndef LOGIN_H
#define LOGIN_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_login_handler(VMContext *vm, LexerContext *lex);
void stmt_login_register(void);

#endif // LOGIN_H
