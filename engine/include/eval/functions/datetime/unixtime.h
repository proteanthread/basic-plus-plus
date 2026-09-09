// FILENAME: unixtime.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (unixtime.c)
// NEEDS: libcore (types.h)
// Declares the UNIXTIME built-in function interface in BASIC++.

#ifndef UNIXTIME_H
#define UNIXTIME_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_unixtime_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_unixtime_register(void);

#endif // UNIXTIME_H
