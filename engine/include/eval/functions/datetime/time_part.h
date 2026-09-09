// FILENAME: time_part.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c, time_part.c)
// NEEDS: libkernel (types.h), libengine (eval.h)
// Declares the TIME_PART / TIMEPART built-in function in BASIC++.

#ifndef TIME_PART_H
#define TIME_PART_H

#include "types/types.h"
#include "eval/eval.h"

void func_time_part_register(void);
BValue func_time_part_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // TIME_PART_H
