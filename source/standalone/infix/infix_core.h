/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: infix_core.h
 * Subsystem: Standalone Infix Math & Expression Parser
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses and evaluates algebraic infix math expression strings
 *    using operator precedence (Shunting-yard algorithm).
 *
 * 2. WHAT TO EXPECT:
 *    Precedence-based parsing. Pure floating-point math evaluation.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Supported operator tokens, priority weights.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Parsing hierarchy logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check parentheses balance and token validity on parse error.
 * ===================================================================== */

#ifndef STANDALONE_INFIX_CORE_H
#define STANDALONE_INFIX_CORE_H

#include <stddef.h>

typedef double (*InfixVarResolver)(void *user_data, const char *name, int *found);
typedef double (*InfixFuncResolver)(void *user_data, const char *name, double arg, int *found);

int infix_core_evaluate(const char *expr, InfixVarResolver var_cb, InfixFuncResolver func_cb, void *user_data, double *out_val);

#endif /* STANDALONE_INFIX_CORE_H */
