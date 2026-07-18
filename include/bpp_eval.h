/**
 * @file bpp_eval.h
 * @brief Iterative Expression Evaluator API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares expression parsing and evaluation routines using the Shunting-Yard algorithm.
 * - Why it exists: Processes math, string, and relational expressions (e.g. A + B * C > D$) without C stack recursion,
 *   supporting operator precedence and dynamic variable resolution.
 * - Why it works this way: It parses and evaluates inline, using stacks allocated in the transient scratch arena,
 *   which guarantees zero host stack footprint and prevents recursive compiler crashes.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Precedence table, operator types, or function calls.
 * - What cannot be changed: The iterative design constraints (no recursive C-level function calls during parse).
 * - What to expect: Consumes tokens up to the end of the expression and returns the final BValue.
 * - What to do if something breaks: If operator precedence fails (e.g. addition precedes multiplication),
 *   trace the get_precedence lookup and operator pop loop.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Token streams are valid. Variables have defined types.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new operators (e.g. exponentiation, bitwise operations)
 *   by modifying precedence lookups and execution cases.
 * - How to write external extensions: Custom function calls (e.g. user-defined DEF FN or SUB functions)
 *   can be evaluated by integrating them into the token dispatch loop.
 */

#ifndef BPP_EVAL_H
#define BPP_EVAL_H

#include "bpp_types.h"
#include "bpp_lexer.h"
#include "bpp_vm.h"

/**
 * @brief Parse and evaluate a BASIC expression from the token stream.
 * @param vm Opaque VMContext pointer (used for variable and string lookup).
 * @param lex Opaque LexerContext pointer to pull tokens from.
 * @param out_err Error block pointer to populate on failure.
 * @return BValue representing the evaluation result.
 */
BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *err);

/**
 * @brief Invokes a user-defined function or method
 */
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);

#endif /* BPP_EVAL_H */
