/*
 * dialect_aint.c -- Apple II Integer BASIC
 *
 * Steve Wozniak, 1977. Hand-assembled in 6502 machine code
 * without an assembler. 8KB ROM, integer only, '>' prompt.
 * LET is mandatory. No floating point, no DATA/READ.
 *
 * Lo-res graphics: GR, COLOR=n, PLOT, HLIN, VLIN, SCRN(x,y)
 * (40x48, 16 colors) -- not yet implemented.
 *
 * TODO: GR/PLOT/HLIN/VLIN/SCRN lo-res graphics
 * TODO: COLOR= assignment syntax
 * TODO: MOD operator
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
    "AINT", DFLAG_AINT
};

void dialect_register_aint(void)
{
    dialect_register(&aint_config);
}
