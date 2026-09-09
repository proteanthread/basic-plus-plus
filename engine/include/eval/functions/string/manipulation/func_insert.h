// FILENAME: func_insert.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_insert.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the INSERT$ string insertion function in BASIC++.

#ifndef FUNC_INSERT_H
#define FUNC_INSERT_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_insert_register(void);
BValue func_insert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_INSERT_H
