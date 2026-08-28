// FILENAME: lcase.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (lcase.c, string_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LCASE built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_LCASE_H
#define EVAL_FUNC_LCASE_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_lcase_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_lcase_register(void);

#endif // EVAL_FUNC_LCASE_H
