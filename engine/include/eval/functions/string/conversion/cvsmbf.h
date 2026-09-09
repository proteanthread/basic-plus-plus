// FILENAME: cvsmbf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cvsmbf.c)
// NEEDS: libcore (types.h)
// Declares the CVSMBF built-in function interface in BASIC++.

#ifndef CVSMBF_H
#define CVSMBF_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_cvsmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_cvsmbf_register(void);

#endif // CVSMBF_H
