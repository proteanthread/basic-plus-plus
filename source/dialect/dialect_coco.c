/*
 * dialect_coco.c -- Color Computer BASIC (Microsoft, 1980)
 *
 * Motorola 6809 -- the best 8-bit CPU nobody used enough.
 * 16KB ROM (Color BASIC + Extended Color BASIC). "OK" prompt.
 * Unlike the C64, the CoCo actually has graphics commands.
 *
 * Missing:
 *   CIRCLE -- the CoCo version has aspect ratio and arc params:
 *   CIRCLE (x,y), radius [,color [,hw [,start, end]]]
 *   The hw param adjusts for non-square pixels. start/end draw
 *   partial arcs. Implement with parametric trig in gfxbuf.
 *
 *   LINE (x1,y1)-(x2,y2), color, BF -- the BF suffix draws a
 *   filled box instead of just a line. Detect B or BF after the
 *   color argument in the parser.
 *
 *   PMODE n, start_page -- graphics resolution/color modes:
 *     PMODE 0: 128x96, 2 colors per page (1.5KB)
 *     PMODE 1: 128x96, 4 colors (3KB)
 *     PMODE 2: 128x192, 2 colors (3KB)
 *     PMODE 3: 128x192, 4 colors (6KB)
 *     PMODE 4: 256x192, 2 colors (6KB)
 *   Map to gfxbuf with corresponding resolution.
 *
 *   CoCo 3 had the GIME chip with 320x200 and 640x200 modes.
 *   Could be a separate dialect (COCO3) that extends this one.
 */

#include "dialect.h"

static const DialectConfig coco_config = {
    DIALECT_COCO,
    "Color Computer BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "OK", 16, 1, 1, 1,
    "COCO", DFLAG_COCO, 0
};

void dialect_register_coco(void)
{
    dialect_register(&coco_config);
}
