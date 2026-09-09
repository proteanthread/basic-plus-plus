// FILENAME: func_remove.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_remove.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the REMOVE$ pattern and substring stripping function in BASIC++.

#ifndef FUNC_REMOVE_H
#define FUNC_REMOVE_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_remove_register(void);
BValue func_remove_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_REMOVE_H
