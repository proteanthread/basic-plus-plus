/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vm_core.h
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

#ifndef STANDALONE_VM_CORE_H
#define STANDALONE_VM_CORE_H

#include "../../config.h"
#include "../value/value_core.h"

/* Execution States */
typedef enum VMCoreState {
    VM_CORE_STOPPED = 0,
    VM_CORE_RUNNING,
    VM_CORE_PAUSED,
    VM_CORE_ERROR,
    VM_CORE_HALTED
} VMCoreState;

/* Stack representation */
typedef struct VMEvalStack {
    BValue items[VM_EVAL_STACK_SIZE];
    int top;
} VMEvalStack;

/* Operations */
void vm_core_eval_init(VMEvalStack *stk);
int vm_core_eval_push(VMEvalStack *stk, BValue val);
int vm_core_eval_pop(VMEvalStack *stk, BValue *out_val);
int vm_core_eval_peek(const VMEvalStack *stk, BValue *out_val);
int vm_core_eval_depth(const VMEvalStack *stk);

#endif /* STANDALONE_VM_CORE_H */
