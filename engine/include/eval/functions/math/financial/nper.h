// FILENAME: nper.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (nper.c)
// NEEDS: libcore (types.h)
// Declares the NPER built-in function interface in BASIC++.

#ifndef NPER_H
#define NPER_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_nper_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_nper_register(void);

#endif // NPER_H
