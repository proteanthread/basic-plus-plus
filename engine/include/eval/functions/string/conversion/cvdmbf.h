// FILENAME: cvdmbf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cvdmbf.c)
// NEEDS: libcore (types.h)
// Declares the CVDMBF built-in function interface in BASIC++.

#ifndef CVDMBF_H
#define CVDMBF_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_cvdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_cvdmbf_register(void);

#endif // CVDMBF_H
