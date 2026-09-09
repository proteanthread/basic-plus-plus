// FILENAME: npv.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (npv.c)
// NEEDS: libcore (types.h)
// Declares the NPV built-in function interface in BASIC++.

#ifndef NPV_H
#define NPV_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_npv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_npv_register(void);

#endif // NPV_H
