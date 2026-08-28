// FILENAME: dot.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dot.c, math_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DOT built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_MATH_DOT_H
#define EVAL_FUNCTIONS_MATH_DOT_H

#include "types/types.h"
#include "vm/vm.h"

BValue func_dot_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_dot_register(void);

#endif // EVAL_FUNCTIONS_MATH_DOT_H
