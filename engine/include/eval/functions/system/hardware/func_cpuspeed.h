// FILENAME: func_cpuspeed.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Function and variable evaluator for CPUSPEED and CPUSPEED$.

#ifndef FUNC_CPUSPEED_H
#define FUNC_CPUSPEED_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_cpuspeed_register(void);

// Evaluates standard function calls or bare variable CPUSPEED / CPUSPEED$
BValue func_cpuspeed_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Evaluates CPUSPEED[...] bracket slicing / sub-channel queries
BValue func_cpuspeed_eval_bracket(VMContext *vm, LexerContext *lex, BppError *err);

// Evaluates CPUSPEED{...} brace property map / configuration queries
BValue func_cpuspeed_eval_brace(VMContext *vm, LexerContext *lex, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_CPUSPEED_H
