// FILENAME: sseg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sseg.c)
// NEEDS: libcore (types.h)
// Declares the SSEG built-in function interface in BASIC++.

#ifndef SSEG_H
#define SSEG_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_sseg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_sseg_register(void);

#endif // SSEG_H
