// FILENAME: curdir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (curdir.c)
// NEEDS: libcore (types.h)
// Declares the CURDIR$ built-in function interface in BASIC++.

#ifndef CURDIR_H
#define CURDIR_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_curdir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_curdir_register(void);

#endif // CURDIR_H
