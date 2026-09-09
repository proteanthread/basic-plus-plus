// FILENAME: ipmt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ipmt.c)
// NEEDS: libcore (types.h)
// Declares the IPMT built-in function interface in BASIC++.

#ifndef IPMT_H
#define IPMT_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_ipmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ipmt_register(void);

#endif // IPMT_H
