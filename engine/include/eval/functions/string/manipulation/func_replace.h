// FILENAME: func_replace.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_replace.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the REPLACE$ string search-and-replace function in BASIC++.

#ifndef FUNC_REPLACE_H
#define FUNC_REPLACE_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_replace_register(void);
BValue func_replace_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_REPLACE_H
