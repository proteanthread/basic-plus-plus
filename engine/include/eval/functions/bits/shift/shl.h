// FILENAME: shl.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (shl.c, sys_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SHL built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_SHL_H
#define EVAL_FUNC_SHL_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_shl_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_shl_register(void);

#endif // EVAL_FUNC_SHL_H
