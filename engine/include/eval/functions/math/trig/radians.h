// FILENAME: radians.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, radians.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the RADIANS built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_MATH_RADIANS_H
#define EVAL_FUNCTIONS_MATH_RADIANS_H

#include "vm/vm.h"
#include "eval/eval.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_radians_register(void);
BValue func_radians_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_MATH_RADIANS_H
