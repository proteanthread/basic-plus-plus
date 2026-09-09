// FILENAME: prod.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (prod.c)
// NEEDS: libcore (types.h)
// Declares the PROD$ built-in function interface in BASIC++.

#ifndef PROD_H
#define PROD_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_prod_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_prod_register(void);

#endif // PROD_H
