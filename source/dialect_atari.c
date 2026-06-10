/*
 * dialect_atari.c -- Atari BASIC (Shepardson Microsystems, 1979)
 *
 * NOT a Microsoft BASIC. Shepardson wrote it from scratch for
 * Atari. Cartridge ROM, tokenized on entry (slow to type, fast
 * to RUN... sort of). 10-column print zones.
 *
 * Atari BASIC has a completely different substring syntax:
 * A$(5,10) instead of MID$(A$,5,6). To support this, the parser
 * needs to detect when a string variable is followed by (m,n) and
 * emit a substring extraction instead of an array access. Check
 * the token type after the variable name in parse_primary().
 *
 * GRAPHICS n (n=0-8) is the mode switcher:
 *   Mode 0: text 40x24 (default)
 *   Mode 3: 40x24, 4 colors (pixmap)
 *   Mode 7: 160x96, 4 colors
 *   Mode 8: 320x192, 1.5 colors (luminance only)
 * Map these to gfxbuf resolutions and color depths.
 *
 * SETCOLOR register, hue, luminance configures the CTIA/GTIA
 * color registers. We'd need a palette table in gfxbuf.
 *
 * SOUND voice, freq, distortion, volume -- 4 independent POKEY
 * voices. More complex than GW-BASIC's SOUND (single tone).
 * Would need a vdev audio backend with polyphonic support.
 *
 * TRAP n works like ON ERROR GOTO n. Already maps to on_error.
 */

#include "dialect.h"

static const DialectConfig atari_config = {
    DIALECT_ATARI_MS,
    "Atari BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    32767, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY",
    10,                     /* narrower zones */
    1, 0, 1,
    "ATRI", DFLAG_ATRI
};

void dialect_register_atari(void)
{
    dialect_register(&atari_config);
}
