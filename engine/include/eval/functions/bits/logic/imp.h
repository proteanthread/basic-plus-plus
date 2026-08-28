// FILENAME: imp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (imp.c, sys_fn.c)
// NEEDS: libengine (eval_internal.h)
// Provides runtime implementation for the IMP built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_IMP_H
#define EVAL_FUNC_IMP_H

#include "eval/eval_internal.h"

BValue func_imp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_imp_register(void);

#endif // EVAL_FUNC_IMP_H
