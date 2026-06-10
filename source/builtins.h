/*
 * ---
 * BASIC++ Interpreter - builtins.h
 * ---
 *
 * Built-in function handler declarations.
 *
 * PURPOSE:
 * Declares all built-in BASIC++ function handlers and the
 * registration function that populates the function registry.
 * These handlers were extracted from parser.c to enable:
 * 1. Registry-based dispatch (instead of inline switch/if)
 * 2. Dialect overrides (swap handlers at runtime)
 * 3. Module extensions (same handler signature)
 * 4. Clean separation of function logic from parsing logic
 *
 * HANDLER SIGNATURE:
 * All handlers use the uniform FuncHandler signature:
 *
 * BValue handler(BValue *args, int argc, void *rt)
 *
 * The 'rt' parameter is an opaque pointer to RuntimeState.
 * Handlers that need runtime services (RND seed, string pool)
 * cast it to (RuntimeState*) internally.
 *
 * CATEGORIES:
 * Handlers are organized by category:
 * FCAT_MATH - ABS, SGN, INT, SQR, SIN, COS, TAN, ATN,
 * LOG, EXP
 * FCAT_STRING - LEN, ASC, VAL, CHR$, STR$, LEFT$, RIGHT$,
 * MID$
 * FCAT_UTIL - RND, SIZE
 *
 * HOW TO ADD A NEW BUILT-IN:
 * 1. Write a handler function in builtins.c with the FuncHandler
 * signature.
 * 2. Declare it in this header.
 * 3. Add a FunctionEntry to the registration table in
 * builtins_register().
 * 4. If needed, add a keyword to lexer.h/c and detok.c.
 *
 * ---
 */

#ifndef BASICPP_BUILTINS_H
#define BASICPP_BUILTINS_H

#include "value.h"

/* --- Registration Function ---
 */

/*
 * builtins_register - Register all built-in functions.
 *
 * Populates the function registry with all standard BASIC++
 * built-in functions. Must be called after funcreg_init() and
 * before any program execution.
 *
 * Boot sequence position: ("Standard Library Init").
 */
void builtins_register(void);

/* --- Math Function Handlers (FCAT_MATH) ---
 * These are pure functions - no side effects, deterministic.
 * Safety: FSAFE_PURE
 */
BValue builtin_abs(BValue *args, int argc, void *rt);
BValue builtin_sgn(BValue *args, int argc, void *rt);
BValue builtin_int_func(BValue *args, int argc, void *rt);
BValue builtin_sqr(BValue *args, int argc, void *rt);
BValue builtin_sin(BValue *args, int argc, void *rt);
BValue builtin_cos(BValue *args, int argc, void *rt);
BValue builtin_tan(BValue *args, int argc, void *rt);
BValue builtin_atn(BValue *args, int argc, void *rt);
BValue builtin_log(BValue *args, int argc, void *rt);
BValue builtin_exp(BValue *args, int argc, void *rt);

/* --- String Function Handlers (FCAT_STRING) ---
 * Pure functions except CHR$ and STR$ which allocate from the
 * string pool (accessed via rt).
 * Safety: FSAFE_PURE (LEN, ASC, VAL) or FSAFE_STATE (CHR$, STR$)
 */
BValue builtin_len(BValue *args, int argc, void *rt);
BValue builtin_asc(BValue *args, int argc, void *rt);
BValue builtin_val(BValue *args, int argc, void *rt);
BValue builtin_chr(BValue *args, int argc, void *rt);
BValue builtin_str(BValue *args, int argc, void *rt);
BValue builtin_left(BValue *args, int argc, void *rt);
BValue builtin_right(BValue *args, int argc, void *rt);
BValue builtin_mid(BValue *args, int argc, void *rt);

/* --- Utility Function Handlers (FCAT_UTIL) ---
 * RND modifies the RNG seed (FSAFE_STATE).
 * SIZE is pure - reads available memory.
 */
BValue builtin_rnd(BValue *args, int argc, void *rt);
BValue builtin_size(BValue *args, int argc, void *rt);

/* --- I/O Function Handlers ---
 * EOF checks file channel end-of-file status.
 */
BValue builtin_eof(BValue *args, int argc, void *rt);

#endif /* BASICPP_BUILTINS_H */
