// FILENAME: math_sum.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, math_sum.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the mathematical SUM built-in function in BASIC++.

#ifndef EVAL_FUNCTIONS_MATH_SUM_H
#define EVAL_FUNCTIONS_MATH_SUM_H

#include "vm/vm.h"
#include "eval/eval.h"

void func_math_sum_register(void);
BValue func_math_sum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_MATH_SUM_H
