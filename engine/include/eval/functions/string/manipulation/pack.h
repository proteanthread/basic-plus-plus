// FILENAME: pack.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pack.c, string_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the PACK built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_PACK_H
#define EVAL_FUNC_PACK_H

#include "vm/vm.h"
#include "eval/eval.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Evaluates the PACK$() serialization built-in function.
// @param vm Pointer to active VM context.
// @param uname Uppercase function name.
// @param arg_count Number of passed arguments.
// @param args Array of argument values.
// @param err Error state output parameter.
// @return BValue holding serialized string.
BValue func_pack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_pack_register(void);

#ifdef __cplusplus
}
#endif

#endif // EVAL_FUNC_PACK_H
