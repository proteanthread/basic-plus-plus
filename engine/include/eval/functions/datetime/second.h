// FILENAME: second.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (second.c)
// NEEDS: libcore (types.h)
// Declares the SECOND built-in function interface in BASIC++.

#ifndef SECOND_H
#define SECOND_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_second_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_second_register(void);

#endif // SECOND_H
