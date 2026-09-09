// FILENAME: year.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (year.c)
// NEEDS: libcore (types.h)
// Declares the YEAR built-in function interface in BASIC++.

#ifndef YEAR_H
#define YEAR_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_year_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_year_register(void);

#endif // YEAR_H
