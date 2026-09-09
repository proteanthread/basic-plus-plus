// FILENAME: hta.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (hta.c)
// NEEDS: libcore (types.h)
// Declares the HTA$ built-in function interface in BASIC++.

#ifndef HTA_H
#define HTA_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_hta_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_hta_register(void);

#endif // HTA_H
