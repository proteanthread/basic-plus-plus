// FILENAME: fv.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (fv.c)
// NEEDS: libcore (types.h)
// Declares the FV built-in function interface in BASIC++.

#ifndef FV_H
#define FV_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_fv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_fv_register(void);

#endif // FV_H
