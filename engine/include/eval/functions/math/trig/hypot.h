// FILENAME: hypot.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (hypot.c, math_fn.c)
// NEEDS: libengine (eval_internal.h)
// Provides runtime implementation for the HYPOT built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_HYPOT_H
#define EVAL_FUNC_HYPOT_H

#include "eval/eval_internal.h"

BValue func_hypot_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_hypot_register(void);

#endif // EVAL_FUNC_HYPOT_H
