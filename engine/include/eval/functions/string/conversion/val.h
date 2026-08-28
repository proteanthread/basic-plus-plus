// FILENAME: val.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, val.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VAL built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_VAL_H
#define EVAL_FUNC_VAL_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_val_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_val_register(void);

#endif // EVAL_FUNC_VAL_H
