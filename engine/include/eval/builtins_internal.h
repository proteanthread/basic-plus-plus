// FILENAME: builtins_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, eval_builtins.c, math_fn.c)
// NEEDED BY: libengine (string_fn.c, sys_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for builtins_internal within BASIC++.
//
// ---- Includes ----

#ifndef BUILTINS_INTERNAL_H
#define BUILTINS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "eval/eval.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Subsystem Category Evaluators ----

bool eval_builtin_math(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res);
bool eval_builtin_string(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res);
bool eval_builtin_sys(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res);
bool eval_builtin_conversion(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err, BValue *out_res);

#endif // BUILTINS_INTERNAL_H
