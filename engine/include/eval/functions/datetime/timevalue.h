// FILENAME: timevalue.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (timevalue.c)
// NEEDS: libcore (types.h)
// Declares the TIMEVALUE built-in function interface in BASIC++.

#ifndef TIMEVALUE_H
#define TIMEVALUE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_timevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_timevalue_register(void);

#endif // TIMEVALUE_H
