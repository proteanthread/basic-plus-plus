/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect_coco.c
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

 // dialect_coco.c -- Color Computer BASIC (Microsoft, 1980)
 //
 // Motorola 6809 -- the best 8-bit CPU nobody used enough.
 // 16KB ROM (Color BASIC + Extended Color BASIC). "OK" prompt.
 // Unlike the C64, the CoCo actually has graphics commands.
 //
 // Missing:
 //   CIRCLE -- the CoCo version has aspect ratio and arc params:
 //   CIRCLE (x,y), radius [,color [,hw [,start, end]]]
 //   The hw param adjusts for non-square pixels. start/end draw
 //   partial arcs. Implement with parametric trig in gfxbuf.
 //
 //   LINE (x1,y1)-(x2,y2), color, BF -- the BF suffix draws a
 //   filled box instead of just a line. Detect B or BF after the
 //   color argument in the parser.
 //
 //   PMODE n, start_page -- graphics resolution/color modes:
 //     PMODE 0: 128x96, 2 colors per page (1.5KB)
 //     PMODE 1: 128x96, 4 colors (3KB)
 //     PMODE 2: 128x192, 2 colors (3KB)
 //     PMODE 3: 128x192, 4 colors (6KB)
 //     PMODE 4: 256x192, 2 colors (6KB)
 //   Map to gfxbuf with corresponding resolution.
 //
 //   CoCo 3 had the GIME chip with 320x200 and 640x200 modes.
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
 //   Could be a separate dialect (COCO3) that extends this one.

#include "dialect.h"

 // coco_apply - Apply Color Computer BASIC configuration.
 //
 // CoCo BASIC is a Microsoft BASIC with graphics and sound:
 //   - PSET, LINE, CIRCLE, PAINT, DRAW for graphics
 //   - PLAY, SOUND for music/sound
 //   - SCREEN, COLOR for mode/palette selection
 //   - INKEY$ for keyboard input
 //   - KEY for function key programming
 //   - ON ERROR GOTO (has_on_error = 1)
 //   - CLS (has_cls = 1), TRON/TROFF (has_tron_troff = 1)
 //   - No WHILE/WEND, no DO/LOOP
 //   - No MERGE/CHAIN
 //   - 16-column print zones
 //   - "OK" prompt (uppercase)
static void coco_apply(void)
{
 // CoCo graphics commands (PSET, PAINT, SCREEN, etc.) are
  // tagged with DFLAG_COCO in the keyword table. Extended
  // Color BASIC adds CIRCLE, LINE, PAINT. CoCo's POINT
  // returns a pixel color (shared with TRS-80 Level I/II
  // block graphics). SET/RESET are block graphics commands
  // shared with TRS-80 (DFLAG_TRS1|DFLAG_TRS2|DFLAG_COCO). 
}

static const DialectConfig coco_config = {
    DIALECT_COCO,
    "Color Computer BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "OK", 16, 1, 1, 1,
    "COCO", DFLAG_COCO, coco_apply
};

void dialect_register_coco(void)
{
    dialect_register(&coco_config);
}
