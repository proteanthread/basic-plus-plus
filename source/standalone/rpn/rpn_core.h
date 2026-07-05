/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: rpn_core.h
 * Subsystem: Reverse Polish Notation Evaluation Stack
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Performs stack evaluation of parsed mathematical expressions.
 *
 * 2. WHAT TO EXPECT:
 *    Pushes/pops variables, evaluating values.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Stack size limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Evaluation routing rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If stack overflows, check expression complexity.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE RPN (REVERSE POLISH NOTATION) CALCULATOR SUBSYSTEM
 * File: rpn_core.h
 * =====================================================================
 * Standard C interface for Forth-like stack evaluator.
 * Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_RPN_CORE_H
#define STANDALONE_RPN_CORE_H

#define RPN_STACK_SIZE 256

typedef struct {
    double stack[RPN_STACK_SIZE];
    int top;
    int active;
} RpnState;

/* Public API */
void rpn_core_init(RpnState *s);
int rpn_core_is_active(RpnState *s);
void rpn_core_set_active(RpnState *s, int on);
int rpn_core_eval_line(RpnState *s, const char *line);

#endif /* STANDALONE_RPN_CORE_H */
