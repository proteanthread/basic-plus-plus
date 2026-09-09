// FILENAME: weekday.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (weekday.c)
// NEEDS: libcore (types.h)
// Declares the WEEKDAY built-in function interface in BASIC++.

#ifndef WEEKDAY_H
#define WEEKDAY_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_weekday_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_weekday_register(void);

#endif // WEEKDAY_H
