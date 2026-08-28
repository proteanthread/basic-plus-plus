// FILENAME: verify_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, verify_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VERIFY_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef VERIFY_FN_H
#define VERIFY_FN_H

#include "types/types.h"
#include "eval/eval_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_verify_register(void);
BValue func_verify_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // VERIFY_FN_H
