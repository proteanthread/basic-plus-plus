// FILENAME: mag.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mag.c, math_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the MAG built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_MATH_MAG_H
#define EVAL_FUNCTIONS_MATH_MAG_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

void func_mag_register(void);
BValue func_mag_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_MATH_MAG_H
