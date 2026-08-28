// FILENAME: log2.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (log2.c, math_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LOG2 built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_LOG2_H
#define EVAL_FUNC_LOG2_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_log2_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_log2_register(void);

#endif // EVAL_FUNC_LOG2_H
