/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vm_core.c
 * Subsystem: Portable VM Evaluation Stack & States
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages the virtual stack operations and execution state transitions.
 *
 * 2. WHAT TO EXPECT:
 *    Fast push/pop routines with explicit boundary/overflow safety bounds.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Stack capacity configuration.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Basic stack push/pop contract signatures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If compiler reports overflow, check VM_EVAL_STACK_SIZE capacity.
 * ===================================================================== */

#include "vm_core.h"

void vm_core_eval_init(VMEvalStack *stk)
{
    if (stk) {
        stk->top = -1;
    }
}

int vm_core_eval_push(VMEvalStack *stk, BValue val)
{
    if (!stk) return -1;
    if (stk->top >= VM_EVAL_STACK_SIZE - 1) {
        return -1;
    }
    stk->items[++stk->top] = val;
    return 0;
}

int vm_core_eval_pop(VMEvalStack *stk, BValue *out_val)
{
    if (!stk || stk->top < 0) {
        return -1;
    }
    if (out_val) {
        *out_val = stk->items[stk->top];
    }
    stk->top--;
    return 0;
}

int vm_core_eval_peek(const VMEvalStack *stk, BValue *out_val)
{
    if (!stk || stk->top < 0) {
        return -1;
    }
    if (out_val) {
        *out_val = stk->items[stk->top];
    }
    return 0;
}

int vm_core_eval_depth(const VMEvalStack *stk)
{
    if (!stk) return 0;
    return stk->top + 1;
}
