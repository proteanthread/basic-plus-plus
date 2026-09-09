// FILENAME: func_shell.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, func_shell.c)
// NEEDS: libengine (eval_internal.h), libkernel (types.h)
// Header for the SHELL$ shell command output capture function in BASIC++.

#ifndef FUNC_SHELL_H
#define FUNC_SHELL_H

#include "types/types.h"
#include "eval/eval_internal.h"

void func_shell_register(void);
BValue func_shell_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // FUNC_SHELL_H
