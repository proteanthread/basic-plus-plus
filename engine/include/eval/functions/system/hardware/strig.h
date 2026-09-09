// FILENAME: strig.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (strig.c)
// NEEDS: libcore (types.h)
// Declares the STRIG built-in function interface in BASIC++.

#ifndef STRIG_H
#define STRIG_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_strig_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_strig_register(void);

#endif // STRIG_H
