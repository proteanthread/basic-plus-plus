// FILENAME: avg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (avg.c, math_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the AVG / MEAN built-in function in BASIC++.

#ifndef EVAL_FUNCTIONS_MATH_AVG_H
#define EVAL_FUNCTIONS_MATH_AVG_H

#include "vm/vm.h"
#include "eval/eval.h"

void func_avg_register(void);
BValue func_avg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_MATH_AVG_H
