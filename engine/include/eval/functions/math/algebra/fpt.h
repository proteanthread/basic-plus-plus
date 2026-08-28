// FILENAME: fpt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (fpt.c, math_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the FPT built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_FPT_H
#define FUNC_FPT_H

#include "types/types.h"
#include "vm/vm.h"

void   func_fpt_register(void);
BValue func_fpt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ipt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_FPT_H
