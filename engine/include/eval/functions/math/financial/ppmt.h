// FILENAME: ppmt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ppmt.c)
// NEEDS: libcore (types.h)
// Declares the PPMT built-in function interface in BASIC++.

#ifndef PPMT_H
#define PPMT_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_ppmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ppmt_register(void);

#endif // PPMT_H
