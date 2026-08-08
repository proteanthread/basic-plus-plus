/**
 * @file stack.c
 * @brief Evaluator value stack lifecycle and push/pop operation handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements evaluator value stack operations (`eval_stack_push()`, `eval_stack_pop()`, `eval_stack_peek()`, `eval_stack_clear()`).
 *
 * 2. WHY IT EXISTS:
 * Provides stack discipline with explicit ownership transfer for `BValue` structs during expression evaluation per Rule #1.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Manages fixed array stack zero-initialized by default; zero/NULLs slot on pop per Rule #1 discipline.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'eval'. Includes "eval/eval_internal.h", "eval/eval.h",
 * "types/types.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Increase max stack capacity or add stack depth inspection helpers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Zeroing slot on pop invariant: popping MUST NULL vacated slot immediately to avoid dangling pointer references (Rule #1).
 *
 * 8. WHAT TO EXPECT:
 * Returns popped BValue or ERR_EVAL_STACK_UNDERFLOW / ERR_EVAL_STACK_OVERFLOW.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check stack pointer index bounds (`top >= capacity`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid non-NULL stack context pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Fixed array buffer allocation.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/eval.h
 * - engine/include/eval/eval_internal.h
 */

/**
 * @file eval_stack.c
 * @brief Evaluator Stack Operations Micro-Library.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements safe Shunting-Yard operand and operator stack management.
 * - Why it exists: Decouples stack manipulation from eval.c.
 * - Why it works this way: Bounded index checks prevent overflow and underflow vulnerabilities.
 *
 * SECTION 2: SAFE ZONES FOR EDITING
 * - Safe to add stack inspection / debugging routines.
 *
 * SECTION 3: CRITICAL INVARIANTS
 * - Stack pointers must never exceed capacity.
 *
 * SECTION 4: EXPECTED SIDE-EFFECTS & BEHAVIORS
 * - Returns true on success, false on stack limit overflow or underflow.
 *
 * SECTION 5: WHAT TO DO IF SOMETHING BREAKS
 * - Verify stack buffer allocation sizes.
 *
 * SECTION 6: ASSUMPTIONS
 * - Pointer inputs are non-NULL.
 *
 * SECTION 7: PORTABILITY CONCERNS
 * - Strictly portable C17.
 *
 * SECTION 8: FUTURE EXPANSIONS
 * - Dynamic arena stack auto-resizing.
 */

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
