// FILENAME: jiffies.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (types.h)
// Declares registration and evaluation handlers for the JIFFIES system variable/function in BASIC++.

#ifndef JIFFIES_H
#define JIFFIES_H

#include "types/types.h"
#include "eval/eval.h"

void func_jiffies_register(void);
BValue func_jiffies_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // JIFFIES_H
