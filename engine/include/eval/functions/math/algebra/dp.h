// FILENAME: dp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dp.c)
// NEEDS: libcore (types.h)
// Declares the DP (Digit Part) built-in function interface in BASIC++.

#ifndef DP_H
#define DP_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_dp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_dp_register(void);

#endif // DP_H
