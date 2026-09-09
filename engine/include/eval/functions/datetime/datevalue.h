// FILENAME: datevalue.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (datevalue.c)
// NEEDS: libcore (types.h)
// Declares the DATEVALUE built-in function interface in BASIC++.

#ifndef DATEVALUE_H
#define DATEVALUE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_datevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_datevalue_register(void);

#endif // DATEVALUE_H
