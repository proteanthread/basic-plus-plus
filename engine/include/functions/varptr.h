// FILENAME: varptr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (varptr.c)
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VARPTR built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FUNCTIONS_VARPTR_H
#define FUNCTIONS_VARPTR_H

#include "types/types.h"
#include "vm/vm.h"
#include "eval/eval.h"

BValue func_varptr_eval(BValue *args, int arg_count, void *rt);
BValue func_varptr_str_eval(BValue *args, int arg_count, void *rt);
void func_varptr_register(void);

#endif // FUNCTIONS_VARPTR_H
