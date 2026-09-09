// FILENAME: xp.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (xp.c)
// NEEDS: libcore (types.h)
// Declares the XP (Exponent Part) built-in function interface in BASIC++.

#ifndef XP_H
#define XP_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_xp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_xp_register(void);

#endif // XP_H
