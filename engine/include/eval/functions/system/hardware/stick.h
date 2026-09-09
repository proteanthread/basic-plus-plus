// FILENAME: stick.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stick.c)
// NEEDS: libcore (types.h)
// Declares the STICK built-in function interface in BASIC++.

#ifndef STICK_H
#define STICK_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_stick_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_stick_register(void);

#endif // STICK_H
