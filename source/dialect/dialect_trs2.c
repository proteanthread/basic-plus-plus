/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_trs2.c
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

 // dialect_trs2.c -- TRS-80 Level II BASIC (Microsoft, 1978)
 //
 // 12KB ROM upgrade. Full Microsoft BASIC with float, DEF FN,
 // string arrays, WHILE/WEND, ON ERROR. Also Model III/4.
 //
 // The big gap here is type suffixes: Level II distinguished A!
 // (single precision) from A# (double). To support this properly
 // the lexer would need to strip the suffix and tag the variable
 // entry in the symbol table with a precision field. See lexer.c
 // near token_read_identifier().
 //
 // SET/RESET/POINT should share the TRS1 implementation once
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
 // that gets written. CLOAD/CSAVE can just alias LOAD/SAVE.

#include "dialect.h"

 // trs2_apply - Apply TRS-80 Level II configuration.
 //
 // Level II is a full Microsoft BASIC with floats, strings,
 // DEF FN, WHILE/WEND, ON ERROR, and type suffixes.
 // SET/RESET/POINT block graphics are available (shared with
 // Level I). CLOAD/CSAVE map to LOAD/SAVE.
 //
 // Key settings:
 //   - has_merge_chain = 0 (Level II did not support MERGE/CHAIN)
 //   - has_while_wend = 1 (added in Level II)
 //   - print_zone_width = 14 (Microsoft standard)
static void trs2_apply(void)
{
 // Level II extended variables use 2-char names (A0-Z9).
  // The parser's named_var support handles this via
  // has_extended_vars = 1. No additional runtime setup
  // needed beyond what the config flags provide. 
}

static const DialectConfig trs2_config = {
    DIALECT_TRS80_L2,
    "TRS-80 Level II BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 1,
    32767, 0, 1, 1, 0, 1, 0, 1, 1, 1,
    "READY", 14, 1, 1, 1,
    "TRS2", DFLAG_TRS2, trs2_apply
};

void dialect_register_trs2(void)
{
    dialect_register(&trs2_config);
}
