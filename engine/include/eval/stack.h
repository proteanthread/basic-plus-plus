// FILENAME: stack.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stack.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for stack within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_EVAL_STACK_H
#define EVAL_EVAL_STACK_H

#include "types/types.h"
#include "lexer/lexer.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    BValue        *values;
    BppTokenType  *operators;
    size_t         val_count;
    size_t         op_count;
    size_t         capacity;
} EvalStack;

void eval_stack_init(EvalStack *st, BValue *val_buf, BppTokenType *op_buf, size_t cap);
bool eval_stack_push_val(EvalStack *st, BValue val);
bool eval_stack_pop_val(EvalStack *st, BValue *out_val);
bool eval_stack_push_op(EvalStack *st, BppTokenType op);
bool eval_stack_pop_op(EvalStack *st, BppTokenType *out_op);

#endif // EVAL_EVAL_STACK_H
