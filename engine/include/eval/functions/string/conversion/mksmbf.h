// FILENAME: mksmbf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mksmbf.c)
// NEEDS: libcore (types.h)
// Declares the MKSMBF$ built-in function interface in BASIC++.

#ifndef MKSMBF_H
#define MKSMBF_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_mksmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_mksmbf_register(void);

#endif // MKSMBF_H
