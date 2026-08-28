// FILENAME: abs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (abs.c, math_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ABS built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_ABS_H
#define EVAL_FUNC_ABS_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_abs_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_abs_register(void);

#endif // EVAL_FUNC_ABS_H
