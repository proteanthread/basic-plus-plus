// FILENAME: ceil.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ceil.c, math_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CEIL built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_CEIL_H
#define FUNC_CEIL_H

#include "vm/vm.h"
#include "types/types.h"

void func_ceil_register(void);
BValue func_ceil_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_CEIL_H
