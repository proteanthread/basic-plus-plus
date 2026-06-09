/*
 * =====================================================================
 * BASIC++ Dialect Profile — ECMA-55 Minimal BASIC (EC55)
 * =====================================================================
 *
 * STANDARD:  ECMA-55 (1978), ISO 6373:1984, ANSI X3.60-1978
 * AUTHOR:    ECMA Technical Committee TC21
 * YEAR:      1978
 *
 * HISTORICAL NOTES:
 *   The first international BASIC standard. ECMA-55 formally
 *   specifies a "Minimal BASIC" — a portable subset that all
 *   conforming implementations must support. It was designed
 *   for program interchange between different BASIC systems.
 *
 *   ECMA-55 is deliberately restrictive: it defines what MUST
 *   work, not what CAN work. A conforming program runs identically
 *   on any ECMA-55 implementation.
 *
 * DIALECT QUIRKS (compared to Microsoft BASIC):
 *   - LET is MANDATORY (no bare assignment: must write LET A=5)
 *   - No WHILE/WEND, no DO/LOOP
 *   - No MERGE/CHAIN
 *   - No CLS, no TRON/TROFF
 *   - No ON ERROR GOTO
 *   - RANDOMIZE supported (but no argument — always seeds randomly)
 *   - Max line number: 99999 (5 digits allowed)
 *   - PRINT zone width: 14 columns (standardized)
 *   - MAT operations NOT included (that's ECMA-116)
 *   - DEF FN limited to single-argument functions
 *   - TAB() function in PRINT
 *   - 6 relational operators: = <> < > <= >=
 *   - Prompt: "READY"
 *
 * WHAT'S IMPLEMENTED:
 *   [x] LET mandatory
 *   [x] IF/THEN (THEN required)
 *   [x] FOR/NEXT with STEP
 *   [x] DATA/READ/RESTORE
 *   [x] DEF FN (single parameter)
 *   [x] DIM arrays (up to 2 dimensions)
 *   [x] GOSUB/RETURN
 *   [x] ON GOTO/GOSUB
 *   [x] PRINT with TAB(), zones
 *   [x] RANDOMIZE
 *   [x] All standard math functions
 *   [x] String variables and functions
 *
 * WHAT'S MISSING (for 100% compliance):
 *   [ ] OPTION BASE 0|1 (array base declaration)
 *       ECMA-55 specifies that OPTION BASE must appear before any
 *       DIM and controls whether arrays start at index 0 or 1.
 *       Fix: Add option_base flag to RuntimeState, check in
 *       DIM handler and array access code.
 *   [ ] Strict line-number-only flow (no labels)
 *   [ ] Exact numeric precision rules (ECMA-55 specifies minimum
 *       6 significant digits for numeric output)
 *   [ ] INPUT prompt restrictions (ECMA-55 requires '? ' prompt)
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit ecma55_config to change feature flags.
 *   2. For OPTION BASE: add has_option_base flag, implement in
 *      parser.c under KW_OPTION handling.
 *   3. Rebuild. Test with: DIALECT "EC55"
 *   4. Validate with ECMA-55 conformance test suite (if available).
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig ecma55_config = {
    DIALECT_ECMA55,                 /* id */
    "ECMA-55 Minimal BASIC",       /* name */
    ':',    /* stmt_separator (we allow ':' for convenience) */
    1,      /* has_then_keyword: YES — THEN is mandatory */
    0,      /* has_let_optional: NO — LET is MANDATORY */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function */
    99999,  /* max_line_number: 5 digits */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    1,      /* has_extended_vars */
    0,      /* has_merge_chain: NO */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "READY",    /* ready_prompt */
    14,         /* print_zone_width: standardized at 14 */
    0,          /* has_on_error: NO */
    0,          /* has_cls: NO */
    0,          /* has_tron_troff: NO */
    "EC55",     /* short_name */
    DFLAG_EC55  /* dialect_flag */
};

void dialect_register_ecma55(void)
{
    dialect_register(&ecma55_config);
}
