 // dialect_trs1.c -- TRS-80 Level I BASIC (1977)
 //
 // Based on Tiny BASIC, upgraded by Steve Leininger for Radio Shack.
 // Z80 at 1.774 MHz, 4KB ROM, 64x16 text. The "Trash-80."
 //
 // Still missing the block graphics (SET/RESET/POINT). The original
 // used 2x3 sub-blocks inside each character cell for 128x48 pseudo
 // pixels. Unicode half-blocks could fake this -- render through
 // gfxbuf at that resolution and map to terminal characters.
 //
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
 // Also missing: MEM function (maps to SIZE internally).

#include "dialect.h"

 // trs1_apply - Apply TRS-80 Level I configuration.
 //
 // Level I was a stripped BASIC with string vars but no string
 // functions beyond basic assignment. SET/RESET/POINT are the
 // block graphics commands. Single-letter variable names only.
 //
 // Key restrictions vs Level II:
 //   - No WHILE/WEND
 //   - No MERGE/CHAIN
 //   - No ON ERROR
 //   - No DIM (only @() arrays, but also supports DIM via flag)
 //   - CLS via specific control character
static void trs1_apply(void)
{
 // Level I's unique behavior is that string variables exist
  // but string functions (LEFT$, MID$, etc.) do not.
  // The has_string_functions = 1 flag allows basic string ops.
  // SET/RESET/POINT are tagged with DFLAG_TRS1 in the keyword
  // table, so they only appear in this dialect's strict mode. 
}

static const DialectConfig trs1_config = {
    DIALECT_TRS80_L1,
    "TRS-80 Level I BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 1,
    32767, 0, 1, 0, 0, 0, 0, 0, 1, 1,
    "READY", 16, 0, 1, 0,
    "TRS1", DFLAG_TRS1, trs1_apply
};

void dialect_register_trs1(void)
{
    dialect_register(&trs1_config);
}
