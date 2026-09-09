// FILENAME: func_clocks.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Function and variable evaluator for CLOCKS and CLOCKS$.

#ifndef FUNC_CLOCKS_H
#define FUNC_CLOCKS_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void func_clocks_register(void);

// Evaluates standard function calls or bare variable CLOCKS / CLOCKS$
BValue func_clocks_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Evaluates CLOCKS[...] bracket slicing / sub-channel queries
BValue func_clocks_eval_bracket(VMContext *vm, LexerContext *lex, BppError *err);

// Evaluates CLOCKS{...} brace property map / configuration queries
BValue func_clocks_eval_brace(VMContext *vm, LexerContext *lex, BppError *err);

#ifdef __cplusplus
}
#endif

#endif // FUNC_CLOCKS_H
