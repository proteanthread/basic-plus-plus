// FILENAME: det.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (det.c, math_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DET built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_MATH_DET_H
#define EVAL_FUNCTIONS_MATH_DET_H

#include "types/types.h"
#include "vm/vm.h"

BValue func_det_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_det_register(void);

#endif // EVAL_FUNCTIONS_MATH_DET_H
