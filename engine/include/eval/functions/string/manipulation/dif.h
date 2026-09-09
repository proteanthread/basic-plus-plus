// FILENAME: dif.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dif.c)
// NEEDS: libcore (types.h)
// Declares the DIF$ built-in function interface in BASIC++.

#ifndef DIF_H
#define DIF_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_dif_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_dif_register(void);

#endif // DIF_H
