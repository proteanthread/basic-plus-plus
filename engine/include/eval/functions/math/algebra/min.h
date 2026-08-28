// FILENAME: min.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, min.c)
// NEEDS: libengine (eval_internal.h)
// Provides runtime implementation for the MIN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_MIN_H
#define EVAL_FUNC_MIN_H

#include "eval/eval_internal.h"

BValue func_min_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_min_register(void);

#endif // EVAL_FUNC_MIN_H
