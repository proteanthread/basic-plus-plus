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

/*
 * asft_apply - Apply AppleSoft BASIC configuration.
 *
 * AppleSoft is a full Microsoft BASIC with floats and strings.
 * Unlike GW-BASIC, it does NOT have WHILE/WEND or DO/LOOP.
 * It DOES have ON ERROR (as ONERR GOTO, aliased).
 * No MERGE/CHAIN. 16-column print zones.
 *
 * Lo-res graphics (GR, COLOR, PLOT, HLIN, VLIN) are shared
 * with Integer BASIC via DFLAG_AINT|DFLAG_ASFT. Hi-res
 * commands (HGR, HPLOT, HCOLOR) are DFLAG_ASFT only.
 */
static void asft_apply(void)
{
 /* AppleSoft's unique bracket prompt ']' and its ONERR GOTO
  * syntax (vs ON ERROR GOTO) are handled via the config
  * struct and keyword aliasing in the lexer. FRE(0) returns
  * free memory -- handled by the FRE keyword tagged with
  * DFLAG_ASFT in the keyword table. */
}

static const DialectConfig asft_config = {
    DIALECT_APPLESOFT,
    "AppleSoft BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "]",                    /* bracket prompt */
    16, 1, 0, 0,
    "ASFT", DFLAG_ASFT, asft_apply
};

void dialect_register_asft(void)
{
    dialect_register(&asft_config);
}
