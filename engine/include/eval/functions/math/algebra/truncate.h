// FILENAME: truncate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, truncate.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the TRUNCATE built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_MATH_TRUNCATE_H
#define EVAL_FUNCTIONS_MATH_TRUNCATE_H

#include "vm/vm.h"
#include "eval/eval.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_truncate_register(void);
BValue func_truncate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNCTIONS_MATH_TRUNCATE_H
