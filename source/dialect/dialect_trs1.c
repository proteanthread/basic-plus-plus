/*
 * dialect_trs1.c -- TRS-80 Level I BASIC (1977)
 *
 * Based on Tiny BASIC, upgraded by Steve Leininger for Radio Shack.
 * Z80 at 1.774 MHz, 4KB ROM, 64x16 text. The "Trash-80."
 *
 * Still missing the block graphics (SET/RESET/POINT). The original
 * used 2x3 sub-blocks inside each character cell for 128x48 pseudo
 * pixels. Unicode half-blocks could fake this -- render through
 * gfxbuf at that resolution and map to terminal characters.
 *
 * Also missing: MEM function (maps to SIZE internally).
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
