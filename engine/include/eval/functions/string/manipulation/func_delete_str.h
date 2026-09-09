// FILENAME: func_delete_str.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_delete_str.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the DELETE$ positional slice deletion function in BASIC++.

#ifndef FUNC_DELETE_STR_H
#define FUNC_DELETE_STR_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_delete_str_register(void);
BValue func_delete_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_DELETE_STR_H
