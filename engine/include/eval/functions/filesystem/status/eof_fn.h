// FILENAME: eof_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eof_fn.c, sys_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the EOF_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_EOF_FN_H
#define EVAL_FUNC_EOF_FN_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_eof_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_eof_fn_register(void);

#endif // EVAL_FUNC_EOF_FN_H
