// FILENAME: day.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (day.c)
// NEEDS: libcore (types.h)
// Declares the DAY built-in function interface in BASIC++.

#ifndef DAY_H
#define DAY_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_day_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_day_register(void);

#endif // DAY_H
