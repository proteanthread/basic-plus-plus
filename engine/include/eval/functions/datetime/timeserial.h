// FILENAME: timeserial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (timeserial.c)
// NEEDS: libcore (types.h)
// Declares the TIMESERIAL built-in function interface in BASIC++.

#ifndef TIMESERIAL_H
#define TIMESERIAL_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_timeserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_timeserial_register(void);

#endif // TIMESERIAL_H
