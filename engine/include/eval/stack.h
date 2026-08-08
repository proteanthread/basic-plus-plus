/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_stack.h
 * @brief Evaluator Stack Operations Micro-Library Header.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares safe operand and operator stack management routines for expression evaluation.
 * - Why it exists: Isolated micro-library for Shunting-Yard evaluation stacks.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Capacity bounds.
 * - What cannot be changed: Stack pointer invariants.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - None.
 */

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

#endif /* EVAL_EVAL_STACK_H */
