// FILENAME: irr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (irr.c)
// NEEDS: libcore (types.h)
// Declares the IRR built-in function interface in BASIC++.

#ifndef IRR_H
#define IRR_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_irr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_irr_register(void);

#endif // IRR_H
