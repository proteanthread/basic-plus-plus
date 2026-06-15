/*
 * dialect_sbasic.c -- Tymshare SUPER BASIC (1968)
 *
 * Mark Alexander and Dan Streng's SUPER BASIC for the SDS 940
 * mainframe (Scientific Data Systems, later Xerox). One of the
 * most advanced BASIC dialects of the late 1960s, extending
 * Dartmouth BASIC with:
 *
 *   - Complex number arithmetic (native type)
 *   - Double precision via # suffix
 *   - JOSS-style postfix modifiers (IF, UNLESS, FOR)
 *   - Extended math: ASIN, ACOS, SINH, COSH, TANH, LOG10, LOG2
 *   - Comparison functions: COMP, PDIF, MIN, MAX
 *   - FOR...BY...TO ordering (step before limit)
 *   - IMAGE statement for formatted output
 *   - Enhanced matrix math (MAT READ, MAT PRINT, etc.)
 *   - File I/O with named channels
 *
 * Tymshare was a pioneering time-sharing company; SUPER BASIC was
 * used via teletype terminals over telephone lines. The dialect
 * influenced later mainframe BASICs and established many features
 * that eventually became standard in ECMA-116 and QBasic.
 *
 * SUPER BASIC requires LET for assignment (like Dartmouth).
 * Statement separator is colon. Max line number is 99999.
 */

#include "dialect.h"

static const DialectConfig sbasic_config = {
    DIALECT_SBASIC,         /* id */
    "SUPER BASIC",          /* name */
    ':',                    /* stmt_separator */
    1,                      /* has_then_keyword */
    0,                      /* has_let_optional -- requires LET */
    1,                      /* has_for_next */
    1,                      /* has_string_vars */
    1,                      /* has_print_hash */
    0,                      /* has_array_at */
    1,                      /* has_rnd_function */
    1,                      /* has_abs_function */
    0,                      /* has_size_function */
    99999,                  /* max_line_number */
    0,                      /* not_eq_is_hash */
    1,                      /* has_data_read */
    0,                      /* has_while_wend -- predates WHILE */
    0,                      /* has_do_loop -- predates DO */
    1,                      /* has_extended_vars */
    0,                      /* has_merge_chain */
    1,                      /* has_float */
    1,                      /* has_dim_arrays */
    1,                      /* has_string_functions */
    "READY",                /* ready_prompt */
    15,                     /* print_zone_width */
    0,                      /* has_on_error -- predates ON ERROR */
    0,                      /* has_cls -- mainframe terminal */
    1,                      /* has_tron_troff */
    "SBAS",                 /* short_name */
    DFLAG_SBAS,             /* dialect_flag */
    0                    /* apply_fn */
};

void dialect_register_sbasic(void)
{
    dialect_register(&sbasic_config);
}
