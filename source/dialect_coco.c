/*
 * dialect_coco.c -- Color Computer BASIC (CoCo 1/2/3)
 *
 * Microsoft Extended Color BASIC, 1980. Motorola 6809 CPU.
 * 16KB ROM (Color BASIC + Extended Color BASIC).
 * "OK" prompt (uppercase). 16-column zones.
 *
 * Built-in graphics (PSET, LINE, CIRCLE, PAINT) and sound
 * (PLAY macro language, SOUND freq/duration). No WHILE/WEND.
 *
 * TODO: CIRCLE with aspect ratio and arc parameters
 * TODO: LINE ,BF filled box mode
 * TODO: PMODE graphics page modes
 * TODO: PCLS/PCOPY
 * TODO: CoCo 3 GIME extensions (HSCREEN, HCOLOR, HPRINT)
 */

#include "dialect.h"

static const DialectConfig coco_config = {
    DIALECT_COCO,
    "Color Computer BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "OK", 16, 1, 1, 1,
    "COCO", DFLAG_COCO
};

void dialect_register_coco(void)
{
    dialect_register(&coco_config);
}
