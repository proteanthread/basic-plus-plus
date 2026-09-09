// FILENAME: paddle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (paddle.c)
// NEEDS: libcore (types.h)
// Declares the PADDLE built-in function interface in BASIC++.

#ifndef PADDLE_H
#define PADDLE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_paddle_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_paddle_register(void);

#endif // PADDLE_H
