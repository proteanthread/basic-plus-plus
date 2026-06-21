/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_c64.c
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

 // dialect_c64.c -- Commodore BASIC v2 (Microsoft, 1977/1982)
 //
 // Licensed by Commodore in 1977, shipped unchanged on the C64
 // in 1982. Five years of hardware advances and they never
 // updated the BASIC. Want sprites? POKE 53248. Want music?
 // POKE 54272. That's just how it was.
 //
 // "READY." with period. 10-column zones. 38911 BASIC BYTES FREE.
 //
 // The interesting emulation targets:
 //   VIC-II registers at $D000-$D02E control sprites, scrolling,
 //   and raster interrupts. A virtual PEEK/POKE memory map through
 //   memmap.c could intercept writes to these addresses and update
 //   a sprite layer in gfxbuf.
 //
 //   SID chip at $D400-$D418 has three oscillators with ADSR
 //   envelopes, ring modulation, and a multi-mode filter. Full
 //   emulation is a big project. Simpler approach: intercept the
 //   frequency and gate registers, feed them to a tone generator.
 //
 //   RND(0) should return the last random value without reseeding.
 //   RND with negative arg seeds with that value. Check the sign
 //   in builtins.c fn_rnd() and add a "last_rnd" field to
 //   RuntimeState.
 //
 //   TI and TI$ -- jiffy clock (1/60s ticks since power on).
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
 //   Map to platform_ticks_ms() and format as HH:MM:SS.

#include "dialect.h"

 // c64_apply - Apply Commodore BASIC v2 configuration.
 //
 // Commodore BASIC v2 is a bare-bones Microsoft BASIC:
 //   - No WHILE/WEND, no DO/LOOP
 //   - No ON ERROR GOTO (has_on_error = 0)
 //   - No MERGE/CHAIN
 //   - No CLS (use PRINT CHR$(147) to clear screen)
 //   - No TRON/TROFF
 //   - CLR clears variables (tagged DFLAG_C64B)
 //   - "READY." prompt with period
 //   - 10-column print zones
 //   - PEEK/POKE for hardware access
 //   - FRE(0) for free memory
 //   - GET/PUT for file I/O
static void c64_apply(void)
{
 // Commodore BASIC v2 has no structured programming features
  // beyond GOSUB/RETURN and FOR/NEXT. All hardware access is
  // through PEEK/POKE. The CLR keyword clears all variables
  // and is tagged with DFLAG_C64B. SYS addr calls machine
  // language subroutines. 
}

static const DialectConfig c64_config = {
    DIALECT_COMMODORE,
    "Commodore BASIC v2",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY.", 10, 0, 0, 0,
    "C64B", DFLAG_C64B, c64_apply
};

void dialect_register_c64(void)
{
    dialect_register(&c64_config);
}
