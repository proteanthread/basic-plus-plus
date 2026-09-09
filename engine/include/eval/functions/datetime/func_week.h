// FILENAME: func_week.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident_builtin.c
// Declarations for WEEK and WEEK$ date functions in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_WEEK_H
#define FUNC_WEEK_H

#include "types/types.h"
#include "eval/eval.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_week_register(void);
BValue func_week_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err);
BValue func_week_str_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_WEEK_H
