// FILENAME: ipt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ipt.c)
// NEEDS: libcore (types.h)
// Declares the IPT (Integer Part) built-in function interface in BASIC++.

#ifndef IPT_H
#define IPT_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_ipt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ipt_register(void);

#endif // IPT_H
