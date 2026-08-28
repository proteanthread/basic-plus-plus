// FILENAME: str_math.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (str_math.c, string_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the STR_MATH built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_STRING_STR_MATH_H
#define EVAL_FUNCTIONS_STRING_STR_MATH_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

void func_str_math_register(void);
BValue func_sum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_dif_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_prod_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_quo_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_place_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_STRING_STR_MATH_H
