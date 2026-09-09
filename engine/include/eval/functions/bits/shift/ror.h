// FILENAME: ror.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ror.c, sys_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the ROR built-in function in BASIC++.

#ifndef EVAL_FUNCTIONS_BITS_ROR_H
#define EVAL_FUNCTIONS_BITS_ROR_H

#include "vm/vm.h"
#include "eval/eval.h"

void func_ror_register(void);
BValue func_ror_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_BITS_ROR_H
