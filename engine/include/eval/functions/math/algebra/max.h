// FILENAME: max.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, max.c)
// NEEDS: libengine (eval_internal.h)
// Provides runtime implementation for the MAX built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_MAX_H
#define EVAL_FUNC_MAX_H

#include "eval/eval_internal.h"

BValue func_max_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_max_register(void);

#endif // EVAL_FUNC_MAX_H
