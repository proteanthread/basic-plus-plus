// FILENAME: hour.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (hour.c)
// NEEDS: libcore (types.h)
// Declares the HOUR built-in function interface in BASIC++.

#ifndef HOUR_H
#define HOUR_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_hour_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_hour_register(void);

#endif // HOUR_H
