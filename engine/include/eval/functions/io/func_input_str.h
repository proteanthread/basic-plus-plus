// FILENAME: func_input_str.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libengine (vm.h, eval.h)
// Declares INPUT$ function interface in BASIC++.

#ifndef FUNC_INPUT_STR_H
#define FUNC_INPUT_STR_H

#include "vm/vm.h"
#include "eval/eval.h"

BValue func_input_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_input_str_register(void);

#endif // FUNC_INPUT_STR_H
