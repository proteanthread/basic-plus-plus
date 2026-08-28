// FILENAME: stack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (stack.h)
// Provides core logic and interface definitions for stack within BASIC++.
//
// ---- Includes ----

#include "eval/stack.h"

void eval_stack_init(EvalStack *st, BValue *val_buf, BppTokenType *op_buf, size_t cap) {
    if (!st) return;
    st->values = val_buf;
    st->operators = op_buf;
    st->val_count = 0;
    st->op_count = 0;
    st->capacity = cap;
}

bool eval_stack_push_val(EvalStack *st, BValue val) {
    if (!st || st->val_count >= st->capacity) return false;
    st->values[st->val_count++] = val;
    return true;
}

bool eval_stack_pop_val(EvalStack *st, BValue *out_val) {
    if (!st || st->val_count == 0) return false;
    if (out_val) *out_val = st->values[--st->val_count];
    return true;
}

bool eval_stack_push_op(EvalStack *st, BppTokenType op) {
    if (!st || st->op_count >= st->capacity) return false;
    st->operators[st->op_count++] = op;
    return true;
}

bool eval_stack_pop_op(EvalStack *st, BppTokenType *out_op) {
    if (!st || st->op_count == 0) return false;
    if (out_op) *out_op = st->operators[--st->op_count];
    return true;
}
