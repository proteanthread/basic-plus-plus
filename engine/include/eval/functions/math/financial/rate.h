// FILENAME: rate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rate.c)
// NEEDS: libcore (types.h)
// Declares the RATE built-in function interface in BASIC++.

#ifndef RATE_H
#define RATE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_rate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_rate_register(void);

#endif // RATE_H
