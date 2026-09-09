// FILENAME: arg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, arg.c)
// NEEDS: libcore, libengine
// Declarations for ARG (Polar Angle / Argument) function (JOSS / RAND P-2922).

#ifndef ARG_H
#define ARG_H

#include "eval/eval.h"

void func_arg_register(void);
BValue func_arg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // ARG_H
