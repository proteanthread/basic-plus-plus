/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_ecma55.c
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

 // dialect_ecma55.c -- ECMA-55 Minimal BASIC (ISO 6373:1984)
 //
 // International standard. The portable subset: if it runs under
 // EC55 strict mode, it'll run on any conforming implementation.
 // LET is mandatory, no WHILE, no CLS, no ON ERROR.
 //
 // Missing: OPTION BASE 0|1 (currently always 0-based). To add
 // it, the DIM handler in parser.c would need to store the base
 // in the array header and offset all index calculations. Also
 // missing: strict numeric precision rules (at least 6 significant
 // digits). We already use double, so this is mostly satisfied,
 // but ECMA-55 has specific rounding behavior in PRINT that we
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
 // don't enforce.

#include "dialect.h"

 // ecma55_apply - Apply ECMA-55 Minimal BASIC configuration.
 //
 // ECMA-55 is a standards-compliant minimal BASIC:
 //   - LET is mandatory (has_let_optional = 0)
 //   - Has strings but only single-letter A$-Z$
 //   - No named/extended variables beyond A-Z, A$-Z$
 //   - No WHILE/WEND, no DO/LOOP, no ON ERROR
 //   - No CLS, no TRON/TROFF
 //   - No MERGE/CHAIN
 //   - Max line number 99999
 //   - Has DATA/READ/RESTORE, DIM, DEF FN
 //   - Floats are available (has_float = 1)
 //   - GOTO into FOR body is a warning in strict mode
static void ecma55_apply(void)
{
 // ECMA-55 mandates LET (enforced by has_let_optional = 0
  // and the parser's LET-required rejection).
  // GOTO-into-FOR warnings are handled in parser_flow.c.
  // Numeric precision defaults to double (>= 6 digits). 
}

static const DialectConfig ecma55_config = {
    DIALECT_ECMA55,
    "ECMA-55 Minimal BASIC",
    ':', 1,
    0, // LET mandatory
    1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY", 14, 0, 0, 0,
    "EC55", DFLAG_EC55, ecma55_apply
};

void dialect_register_ecma55(void)
{
    dialect_register(&ecma55_config);
}
