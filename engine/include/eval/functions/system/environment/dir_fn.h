// FILENAME: dir_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dir_fn.c)
// NEEDS: libcore (types.h)
// Declares the DIR$ built-in function interface in BASIC++.

#ifndef DIR_FN_H
#define DIR_FN_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_dir_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_dir_fn_register(void);

#endif // DIR_FN_H
