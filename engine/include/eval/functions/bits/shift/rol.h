// FILENAME: rol.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rol.c, sys_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the ROL built-in function in BASIC++.

#ifndef EVAL_FUNCTIONS_BITS_ROL_H
#define EVAL_FUNCTIONS_BITS_ROL_H

#include "vm/vm.h"
#include "eval/eval.h"

void func_rol_register(void);
BValue func_rol_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_BITS_ROL_H
