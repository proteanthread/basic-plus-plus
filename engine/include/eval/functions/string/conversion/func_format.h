// FILENAME: func_format.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_format.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the FORMAT$ standalone string formatting function in BASIC++.

#ifndef FUNC_FORMAT_H
#define FUNC_FORMAT_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_format_register(void);
BValue func_format_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_FORMAT_H
