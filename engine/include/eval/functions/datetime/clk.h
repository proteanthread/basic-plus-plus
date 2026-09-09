// FILENAME: clk.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (types.h)
// Declares registration and evaluation handlers for the CLK and CLK$ built-in variables/functions in BASIC++.

#ifndef CLK_H
#define CLK_H

#include "types/types.h"
#include "eval/eval.h"

void func_clk_register(void);
BValue func_clk_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // CLK_H
