/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_atari.c
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

 // dialect_atari.c -- Atari BASIC (Shepardson Microsystems, 1979)
 //
 // NOT a Microsoft BASIC. Shepardson wrote it from scratch for
 // Atari. Cartridge ROM, tokenized on entry (slow to type, fast
 // to RUN... sort of). 10-column print zones.
 //
 // Atari BASIC has a completely different substring syntax:
 // A$(5,10) instead of MID$(A$,5,6). To support this, the parser
 // needs to detect when a string variable is followed by (m,n) and
 // emit a substring extraction instead of an array access. Check
 // the token type after the variable name in parse_primary().
 //
 // GRAPHICS n (n=0-8) is the mode switcher:
 //   Mode 0: text 40x24 (default)
 //   Mode 3: 40x24, 4 colors (pixmap)
 //   Mode 7: 160x96, 4 colors
 //   Mode 8: 320x192, 1.5 colors (luminance only)
 // Map these to gfxbuf resolutions and color depths.
 //
 // SETCOLOR register, hue, luminance configures the CTIA/GTIA
 // color registers. We'd need a palette table in gfxbuf.
 //
 // SOUND voice, freq, distortion, volume -- 4 independent POKEY
 // voices. More complex than GW-BASIC's SOUND (single tone).
 // Would need a vdev audio backend with polyphonic support.
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
 // TRAP n works like ON ERROR GOTO n. Already maps to on_error.

#include "dialect.h"

 // atari_apply - Apply Atari BASIC configuration.
 //
 // Atari BASIC is not Microsoft-derived, so it has different
 // conventions:
 //   - 10-column print zones (narrower than Microsoft's 14)
 //   - TRAP instead of ON ERROR GOTO (maps to on_error_line)
 //   - CLR instead of CLEAR (clear variables)
 //   - No WHILE/WEND, no DO/LOOP
 //   - No MERGE/CHAIN
 //   - TRON/TROFF supported (DFLAG includes ATRI via has_tron_troff)
 //   - Substring syntax A$(5,10) differs from MID$
static void atari_apply(void)
{
 // Atari's TRAP keyword is already aliased to KW_TRAP in the
  // keyword table and handled in pi_parse_on(). CLR maps to
  // KW_CLR and is tagged with DFLAG_ATRI. The GRAPHICS
  // mode command would need a new keyword (KW_GRAPHICS)
  // when platform graphics are implemented. 
}

static const DialectConfig atari_config = {
    DIALECT_ATARI_MS,
    "Atari BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    32767, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY",
    10, // narrower zones
    1, 0, 1,
    "ATRI", DFLAG_ATRI, atari_apply
};

void dialect_register_atari(void)
{
    dialect_register(&atari_config);
}
