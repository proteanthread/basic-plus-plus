// FILENAME: month.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (month.c)
// NEEDS: libcore (types.h)
// Declares the MONTH built-in function interface in BASIC++.

#ifndef MONTH_H
#define MONTH_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_month_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_month_register(void);

#endif // MONTH_H
