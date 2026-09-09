// FILENAME: fpt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (fpt.c)
// NEEDS: libcore (types.h)
// Declares the FPT (Fractional Part) built-in function interface in BASIC++.

#ifndef FPT_H
#define FPT_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_fpt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_fpt_register(void);

#endif // FPT_H
