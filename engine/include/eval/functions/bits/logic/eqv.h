// FILENAME: eqv.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eqv.c, sys_fn.c)
// NEEDS: libengine (eval_internal.h)
// Provides runtime implementation for the EQV built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_EQV_H
#define EVAL_FUNC_EQV_H

#include "eval/eval_internal.h"

BValue func_eqv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_eqv_register(void);

#endif // EVAL_FUNC_EQV_H
