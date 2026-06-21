/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_sbasic.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    BASIC historical dialect configuration profiles and strict execution filters.
 *
 * 2. WHAT TO EXPECT:
 *    Dialect configurations switch prompt layouts, printing zone widths, statement separators, and active parsing tokens.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Dialect parameters (READY prompt, separator char), addition of custom dialects.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Dialect lookup logic, strict-mode keyword mask validation.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check dialect switches. If syntax is rejected, check if STRICT mode is enabled or keyword is filtered out.
 * ===================================================================== */

 // dialect_sbasic.c -- Tymshare SUPER BASIC (1968)
 //
 // Mark Alexander and Dan Streng's SUPER BASIC for the SDS 940
 // mainframe (Scientific Data Systems, later Xerox). One of the
 // most advanced BASIC dialects of the late 1960s, extending
 // Dartmouth BASIC with:
 //
 //   - Complex number arithmetic (native type)
 //   - Double precision via # suffix
 //   - JOSS-style postfix modifiers (IF, UNLESS, FOR)
 //   - Extended math: ASIN, ACOS, SINH, COSH, TANH, LOG10, LOG2
 //   - Comparison functions: COMP, PDIF, MIN, MAX
 //   - FOR...BY...TO ordering (step before limit)
 //   - IMAGE statement for formatted output
 //   - Enhanced matrix math (MAT READ, MAT PRINT, etc.)
 //   - File I/O with named channels
 //
 // Tymshare was a pioneering time-sharing company; SUPER BASIC was
 // used via teletype terminals over telephone lines. The dialect
 // influenced later mainframe BASICs and established many features
 // that eventually became standard in ECMA-116 and QBasic.
 //
 // SUPER BASIC requires LET for assignment (like Dartmouth).
//
// HOW TO CUSTOMIZE:
//   Each flag (has_for_next, has_string_vars, etc.) enables or
//   disables a language feature. Set to 1 to enable, 0 to disable.
//   See dialect.c for the complete flag reference.
//
// FINE-TUNING:
//   Adjust max_line_number, max_string_length, and array_base
//   to match the historical limits of this BASIC dialect.
//
// HOW TO EXTEND:
//   To add a new feature flag to this dialect:
//   1. Add the flag to DialectConfig in dialect.h.
//   2. Set its default value in this file's init function.
//   3. Check it with dialect_check_feature() in the parser.
 // Statement separator is colon. Max line number is 99999.

#include "dialect.h"

 // sbasic_apply - Apply SUPER BASIC configuration.
 //
 // SUPER BASIC is a 1960s mainframe BASIC with advanced features:
 //   - Complex number arithmetic (COMPLEX keyword)
 //   - Extended math functions (ASIN, ACOS, SINH, COSH, TANH, etc.)
 //   - JOSS-style UNLESS keyword (postfix conditional)
 //   - FOR...BY...TO syntax (step before limit, BY keyword)
 //   - IMAGE statement for formatted output
 //   - MAT matrix operations
 //   - SWAP, RANDOMIZE, USING
 //   - ELSE keyword available
 //   - LET mandatory (Dartmouth style)
 //   - No WHILE/WEND (predates structured BASIC)
 //   - No DO/LOOP
 //   - No MERGE/CHAIN
 //   - No ON ERROR (predates error trapping)
 //   - No CLS (mainframe terminal)
 //   - TRON/TROFF trace available
 //   - 15-column print zones
static void sbasic_apply(void)
{
 // SUPER BASIC's unique features (COMPLEX arithmetic,
  // UNLESS/BY modifiers, FOR...BY...TO ordering) are
  // implemented in the parser and tagged with DFLAG_SBAS
  // in the keyword table. The extended math functions
  // (ASIN, ACOS, SINH, COSH, TANH, LOG10, LOG2) are
  // tagged DFLAG_ALL since they are BASIC++ native features
  // inspired by SUPER BASIC but available in all dialects. 
}

static const DialectConfig sbasic_config = {
    DIALECT_SBASIC, // id
    "SUPER BASIC", // name
    ':', // stmt_separator
    1, // has_then_keyword
    0, // has_let_optional -- requires LET
    1, // has_for_next
    1, // has_string_vars
    1, // has_print_hash
    0, // has_array_at
    1, // has_rnd_function
    1, // has_abs_function
    0, // has_size_function
    99999, // max_line_number
    0, // not_eq_is_hash
    1, // has_data_read
    0, // has_while_wend -- predates WHILE
    0, // has_do_loop -- predates DO
    1, // has_extended_vars
    0, // has_merge_chain
    1, // has_float
    1, // has_dim_arrays
    1, // has_string_functions
    "READY", // ready_prompt
    15, // print_zone_width
    0, // has_on_error -- predates ON ERROR
    0, // has_cls -- mainframe terminal
    1, // has_tron_troff
    "SBAS", // short_name
    DFLAG_SBAS, // dialect_flag
    sbasic_apply // apply_fn
};

void dialect_register_sbasic(void)
{
    dialect_register(&sbasic_config);
}
