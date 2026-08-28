// FILENAME: complex_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (complex_fn.c, math_fn.c)
// NEEDS: libengine (eval_internal.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the COMPLEX_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef COMPLEX_FN_H
#define COMPLEX_FN_H

#include "types/types.h"
#include "eval/eval_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_complex_register(void);
BValue func_complex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // COMPLEX_FN_H
