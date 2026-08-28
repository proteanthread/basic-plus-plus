// FILENAME: shuffle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (shuffle.c, string_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SHUFFLE built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_STRING_SHUFFLE_H
#define EVAL_FUNCTIONS_STRING_SHUFFLE_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_shuffle_register(void);
BValue func_shuffle_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_STRING_SHUFFLE_H
