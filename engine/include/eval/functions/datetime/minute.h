// FILENAME: minute.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (minute.c)
// NEEDS: libcore (types.h)
// Declares the MINUTE built-in function interface in BASIC++.

#ifndef MINUTE_H
#define MINUTE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_minute_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_minute_register(void);

#endif // MINUTE_H
