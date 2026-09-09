// FILENAME: sum.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sum.c)
// NEEDS: libcore (types.h)
// Declares the SUM$ built-in function interface in BASIC++.

#ifndef SUM_H
#define SUM_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_sum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_sum_register(void);

#endif // SUM_H
