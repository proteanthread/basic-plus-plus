/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_patb.c
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

 // dialect_patb.c -- Palo Alto Tiny BASIC (Li-Chen Wang, 1976)
 //
 // The original hobbyist BASIC. Dr. Dobb's Journal, Vol. 1 No. 1.
 // 2KB interpreter for 4KB systems. Integer-only, 26 vars, no strings.
 //
 // Unique syntax: ';' separates statements, '#' means not-equal,
 // IF lacks THEN, arrays use @(). PRINT # sets field width.
 //
 // Modifying this dialect:
 //   Change flags below and rebuild. If you need new keywords, add them
 //   to lexer.h/lexer.c with DFLAG_PATB, then handle in parser.c.
 //   Example: SIZE could be made to return actual pool usage by hooking
 //   into mem_pool_usage() -- see memory.h.
 //
 // Not yet done:
 //   - @() should share memory with the variable pool (original behavior
 //     used the space between the program and the stack). Could be done
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
 //     by pointing @() into RuntimeState.named_vars directly.

#include "dialect.h"

 // patb_apply - Apply Palo Alto Tiny BASIC configuration.
 //
 // PATB is the most restrictive dialect:
 //   - Integer-only: no floating point (has_float = 0)
 //   - No string variables (has_string_vars = 0)
 //   - No named variables (has_extended_vars = 0)
 //   - No DATA/READ (has_data_read = 0)
 //   - No WHILE/WEND or DO/LOOP
 //   - No DIM arrays (uses @() instead)
 //   - No MERGE/CHAIN
 //   - No ON ERROR
 //   - Semicolon statement separator
 //   - '#' is the not-equal operator
 //   - No THEN keyword required after IF
 //
 // Runtime effects enforced via config flags; the apply callback
 // logs the dialect personality for debugging.
static void patb_apply(void)
{
 // All restrictions enforced via config flags.
  // PATB's integer-only mode is handled by has_float = 0
  // and the parser's dialect_check_feature() gating.
  // No additional runtime reconfiguration needed. 
}

static const DialectConfig patb_config = {
    DIALECT_TINY_BASIC,
    "Palo Alto Tiny BASIC",
    ';', // unique semicolon separator
    0, // no THEN keyword
    1, 1, 0,
    1, // PRINT # field width
    1, // @() array syntax
    1, 1, 1,
    32767, 1, // max line; '#' means not-equal
    0, 1, 1, 0,
    1, 0, 0, 0,
    "READY", 8, 0, 0, 0,
    "PATB", DFLAG_PATB, patb_apply
};

void dialect_register_patb(void)
{
    dialect_register(&patb_config);
}
