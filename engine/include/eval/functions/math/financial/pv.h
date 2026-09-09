// FILENAME: pv.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pv.c)
// NEEDS: libcore (types.h)
// Declares the PV built-in function interface in BASIC++.

#ifndef PV_H
#define PV_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_pv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_pv_register(void);

#endif // PV_H
