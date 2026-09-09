// FILENAME: func_exec.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_exec.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the EXEC$ direct process execution function in BASIC++.

#ifndef FUNC_EXEC_H
#define FUNC_EXEC_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_exec_register(void);
BValue func_exec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_EXEC_H
