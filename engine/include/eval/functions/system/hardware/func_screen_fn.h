// FILENAME: func_screen_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libengine (vm.h, eval.h)
// Declares SCREEN function interface in BASIC++.

#ifndef FUNC_SCREEN_FN_H
#define FUNC_SCREEN_FN_H

#include "vm/vm.h"
#include "eval/eval.h"

BValue func_screen_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_screen_fn_register(void);

#endif // FUNC_SCREEN_FN_H
