/*
 * dialect_trs1.c -- TRS-80 Level I BASIC
 *
 * Steve Leininger, 1977. Based on Tiny BASIC.
 * 4KB ROM, Z80 @ 1.774 MHz, 64x16 text, 128x48 block graphics.
 * Integer-only, limited strings (A$ only reliable).
 *
 * TODO: SET(x,y)/RESET(x,y)/POINT(x,y) block graphics
 * TODO: MEM function
 */

#include "dialect.h"

static const DialectConfig trs1_config = {
    DIALECT_TRS80_L1,
    "TRS-80 Level I BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 1,
    32767, 0, 1, 0, 0, 0, 0, 0, 1, 1,
    "READY", 16, 0, 1, 0,
    "TRS1", DFLAG_TRS1
};

void dialect_register_trs1(void)
{
    dialect_register(&trs1_config);
}
