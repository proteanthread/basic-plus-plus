// FILENAME: func_neg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident.c, ast_parse_expr.c
// Declarations for NEG arithmetic negation function in BASIC++.
//
// ---- Includes ----

#ifndef FUNC_NEG_H
#define FUNC_NEG_H

#include "types/types.h"
#include "eval/eval.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_neg_register(void);
BValue func_neg_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_NEG_H
