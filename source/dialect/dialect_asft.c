/*
 * dialect_asft.c -- AppleSoft BASIC (Microsoft for Apple, 1978)
 *
 * 12KB ROM replacing Wozniak's Integer BASIC on the Apple II+.
 * ']' prompt. ONERR GOTO (not ON ERROR GOTO -- keyword alias).
 *
 * Hi-res graphics are the big missing feature here:
 *   HGR -- init page 1 at $2000 (280x192, 6 colors via NTSC)
 *   HGR2 -- init page 2 at $4000
 *   HPLOT x,y -- set pixel
 *   HPLOT TO x,y -- draw line from last position (Bresenham)
 *   HCOLOR=n -- set color (0-7, NTSC artifact colors)
 *
 *   Implementation: gfxbuf_set_mode(280, 192, 6) for HGR.
 *   NTSC artifact colors are weird -- they depend on whether
 *   the pixel is on an even or odd column. Approximate with
 *   a 6-entry palette: black, green, violet, white, orange, blue.
 *   HPLOT TO needs a "last x, last y" in the runtime state.
 *
 * Lo-res shares commands with Integer BASIC (GR, PLOT, etc).
 * Both dialects tag those keywords with DFLAG_AINT|DFLAG_ASFT.
 */

#include "dialect.h"

static const DialectConfig asft_config = {
    DIALECT_APPLESOFT,
    "AppleSoft BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "]",                    /* bracket prompt */
    16, 1, 0, 0,
    "ASFT", DFLAG_ASFT, 0
};

void dialect_register_asft(void)
{
    dialect_register(&asft_config);
}
