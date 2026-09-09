// FILENAME: ath.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ath.c)
// NEEDS: libcore (types.h)
// Declares the ATH$ built-in function interface in BASIC++.

#ifndef ATH_H
#define ATH_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_ath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ath_register(void);

#endif // ATH_H
