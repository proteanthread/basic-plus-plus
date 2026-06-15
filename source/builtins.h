/*
 * ---
 * BASIC++ Interpreter - builtins.h
 * ---
 *
 * Built-in function handler declarations.
 *
 * All handlers use the uniform FuncHandler signature:
 *
 * BValue handler(BValue *args, int argc, void *rt)
 *
 * The 'rt' parameter is an opaque pointer to RuntimeState.
 * Handlers that need runtime services (RND seed, string pool)
 * cast it to (RuntimeState*) internally.
 *
 * Handler implementations are organized by category across
 * separate source files for modularity:
 *
 *   builtins_math.c     - Arithmetic / Math
 *   builtins_string.c   - String Functions
 *   builtins_fileio.c   - File Input-Output
 *   builtins_io.c       - Input / Output
 *   builtins_memory.c   - Memory
 *   builtins_system.c   - System / Environment
 *   builtins_graphics.c - Graphics
 *   builtins.c          - Registration table only
 *
 * HOW TO ADD A NEW BUILT-IN:
 * 1. Write a handler in the appropriate category file.
 * 2. Declare it in this header.
 * 3. Add a FunctionEntry to the registration table in
 *    builtins_register() in builtins.c.
 * 4. If needed, add a keyword to lexer.h/c and detok.c.
 *
 * ---
 */

#ifndef BASICPP_BUILTINS_H
#define BASICPP_BUILTINS_H

#include "value.h"

/* --- Registration Function ---
 */
void builtins_register(void);

/* --- Arithmetic / Math (builtins_math.c) ---
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
BValue builtin_fix(BValue *args, int argc, void *rt);
BValue builtin_complex(BValue *args, int argc, void *rt);
BValue builtin_real(BValue *args, int argc, void *rt);
BValue builtin_imag(BValue *args, int argc, void *rt);
BValue builtin_conj(BValue *args, int argc, void *rt);
BValue builtin_cabs(BValue *args, int argc, void *rt);
BValue builtin_min(BValue *args, int argc, void *rt);
BValue builtin_max(BValue *args, int argc, void *rt);
BValue builtin_avg(BValue *args, int argc, void *rt);
BValue builtin_med(BValue *args, int argc, void *rt);
BValue builtin_round(BValue *args, int argc, void *rt);
BValue builtin_asin(BValue *args, int argc, void *rt);
BValue builtin_acos(BValue *args, int argc, void *rt);
BValue builtin_sinh(BValue *args, int argc, void *rt);
BValue builtin_cosh(BValue *args, int argc, void *rt);
BValue builtin_tanh(BValue *args, int argc, void *rt);
BValue builtin_log10(BValue *args, int argc, void *rt);
BValue builtin_log2(BValue *args, int argc, void *rt);
BValue builtin_comp(BValue *args, int argc, void *rt);
BValue builtin_pdif(BValue *args, int argc, void *rt);
BValue builtin_pi(BValue *args, int argc, void *rt);
BValue builtin_rnd(BValue *args, int argc, void *rt);

/* --- String Functions (builtins_string.c) ---
 */
BValue builtin_len(BValue *args, int argc, void *rt);
BValue builtin_asc(BValue *args, int argc, void *rt);
BValue builtin_val(BValue *args, int argc, void *rt);
BValue builtin_chr(BValue *args, int argc, void *rt);
BValue builtin_str(BValue *args, int argc, void *rt);
BValue builtin_left(BValue *args, int argc, void *rt);
BValue builtin_right(BValue *args, int argc, void *rt);
BValue builtin_mid(BValue *args, int argc, void *rt);
BValue builtin_instr(BValue *args, int argc, void *rt);
BValue builtin_space(BValue *args, int argc, void *rt);
BValue builtin_string_func(BValue *args, int argc, void *rt);
BValue builtin_hex(BValue *args, int argc, void *rt);
BValue builtin_oct(BValue *args, int argc, void *rt);
BValue builtin_bin(BValue *args, int argc, void *rt);

/* --- File Input-Output (builtins_fileio.c) ---
 */
BValue builtin_eof(BValue *args, int argc, void *rt);
BValue builtin_lof(BValue *args, int argc, void *rt);
BValue builtin_cvi(BValue *args, int argc, void *rt);
BValue builtin_cvs(BValue *args, int argc, void *rt);
BValue builtin_cvd(BValue *args, int argc, void *rt);
BValue builtin_exists(BValue *args, int argc, void *rt);
BValue builtin_filesize(BValue *args, int argc, void *rt);
BValue builtin_filemod(BValue *args, int argc, void *rt);

/* --- Input / Output (builtins_io.c) ---
 */
BValue builtin_csrlin(BValue *args, int argc, void *rt);

/* --- Memory (builtins_memory.c) ---
 */
BValue builtin_peek(BValue *args, int argc, void *rt);
BValue builtin_size(BValue *args, int argc, void *rt);

/* --- System / Environment (builtins_system.c) ---
 */
BValue builtin_environ(BValue *args, int argc, void *rt);

/* --- Graphics (builtins_graphics.c) ---
 */
BValue builtin_point(BValue *args, int argc, void *rt);

#endif /* BASICPP_BUILTINS_H */
