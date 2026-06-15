/*
 * dialect_aint.c -- Apple II Integer BASIC (Wozniak, 1977)
 *
 * Hand-assembled in 6502 without an assembler. 8KB ROM. Fast
 * (integer-only), quirky, '>' prompt. LET mandatory.
 * No floats, no DATA/READ -- not even Microsoft BASIC.
 *
 * The interesting gaps are the graphics commands:
 *   GR switches to lo-res (40x48, 16 colors, bottom 4 lines text).
 *   COLOR=n sets the plot color (0-15, mapped to Apple NTSC palette).
 *   PLOT x,y sets a single lo-res pixel.
 *   HLIN x1,x2 AT y -- horizontal line fill.
 *   VLIN y1,y2 AT x -- vertical line fill.
 *   SCRN(x,y) returns the color at a pixel.
 *
 *   All of these map cleanly to gfxbuf.c. GR initializes a 40x48
 *   buffer; PLOT/HLIN/VLIN are just pixel writes; SCRN reads back.
 *   The parser needs KW_GR, KW_PLOT, KW_HLIN, KW_VLIN, KW_SCRN
 *   in lexer.h with DFLAG_AINT|DFLAG_ASFT.
 *
 * MOD operator: would be a simple case in parse_expression, emit
 * fmod() for floats or % for ints.
 */

#include "dialect.h"

static const DialectConfig aint_config = {
    DIALECT_APPLE_INT,
    "Apple II Integer BASIC",
    ':', 1,
    0,                      /* LET mandatory */
    1, 0, 0, 0, 1, 1, 0,
    32767, 0,
    0,                      /* no DATA/READ */
    0, 0, 0, 0, 0, 1, 0,
    ">", 16, 0, 0, 0,
    "AINT", DFLAG_AINT, 0
};

void dialect_register_aint(void)
{
    dialect_register(&aint_config);
}
