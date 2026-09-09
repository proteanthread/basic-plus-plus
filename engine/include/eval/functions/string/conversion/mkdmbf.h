// FILENAME: mkdmbf.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mkdmbf.c)
// NEEDS: libcore (types.h)
// Declares the MKDMBF$ built-in function interface in BASIC++.

#ifndef MKDMBF_H
#define MKDMBF_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_mkdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_mkdmbf_register(void);

#endif // MKDMBF_H
