/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_qbasic.c
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

 // dialect_qbasic.c -- QBasic (Microsoft, 1991)
 //
 // Shipped with MS-DOS 5.0+. Interpreter-only subset of QuickBASIC
 // 4.5. Line numbers optional, SUB/FUNCTION, SELECT CASE, DO/LOOP.
 // Came with GORILLAS.BAS and NIBBLES.BAS.
 //
 // Missing pieces:
 //   Full SUB/FUNCTION scoping -- QBasic isolates local variables
 //   from the main program. Current DEF FN doesn't do that. Need
 //   a proper call frame on the VM stack (runtime.c) with its own
 //   symbol table. Look at how GOSUB works and add a var frame.
 //
 //   REDIM PRESERVE -- resize an array without losing contents.
 //   Would need realloc in the array backing store (runtime.c,
 //   near rt_dim_array).
 //
 //   SCREEN modes 7-13 (EGA/VGA): these map to gfxbuf resolutions.
 //   SCREEN 12 = 640x480x16, SCREEN 13 = 320x200x256. The gfxbuf
 //   already supports arbitrary resolutions; you'd just add the
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
 //   mode table and palette.

#include "dialect.h"

 // qbasic_apply - Apply QBasic configuration.
 //
 // QBasic is the most structured Microsoft BASIC:
 //   - SUB/FUNCTION with local scope
 //   - SELECT CASE/END SELECT
 //   - DO/LOOP/UNTIL/WHILE (all forms)
 //   - CONST declarations
 //   - REDIM for dynamic arrays
 //   - Line numbers optional
 //   - Full type system (DEFINT/DEFSNG/DEFDBL/DEFSTR)
 //   - SHARED/STATIC variable declarations
static void qbasic_apply(void)
{
 // QBasic's structured features (SUB, FUNCTION, SELECT,
  // DO/LOOP) are all handled by the parser and tagged with
  // DFLAG_QBAS in the keyword table. The TYPE/END TYPE
  // user-defined type system uses parser_struct.c. 
}

static const DialectConfig qbasic_config = {
    DIALECT_QBASIC,
    "QBasic (subset)",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 1, 1,
    "QBAS", DFLAG_QBAS, qbasic_apply
};

void dialect_register_qbasic(void)
{
    dialect_register(&qbasic_config);
}
