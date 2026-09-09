// FILENAME: pmt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pmt.c)
// NEEDS: libcore (types.h)
// Declares the PMT built-in function interface in BASIC++.

#ifndef PMT_H
#define PMT_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_pmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_pmt_register(void);

#endif // PMT_H
