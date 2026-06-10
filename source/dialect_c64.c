/*
 * dialect_c64.c -- Commodore BASIC v2 (Microsoft, 1977/1982)
 *
 * Licensed by Commodore in 1977, shipped unchanged on the C64
 * in 1982. Five years of hardware advances and they never
 * updated the BASIC. Want sprites? POKE 53248. Want music?
 * POKE 54272. That's just how it was.
 *
 * "READY." with period. 10-column zones. 38911 BASIC BYTES FREE.
 *
 * The interesting emulation targets:
 *   VIC-II registers at $D000-$D02E control sprites, scrolling,
 *   and raster interrupts. A virtual PEEK/POKE memory map through
 *   memmap.c could intercept writes to these addresses and update
 *   a sprite layer in gfxbuf.
 *
 *   SID chip at $D400-$D418 has three oscillators with ADSR
 *   envelopes, ring modulation, and a multi-mode filter. Full
 *   emulation is a big project. Simpler approach: intercept the
 *   frequency and gate registers, feed them to a tone generator.
 *
 *   RND(0) should return the last random value without reseeding.
 *   RND with negative arg seeds with that value. Check the sign
 *   in builtins.c fn_rnd() and add a "last_rnd" field to
 *   RuntimeState.
 *
 *   TI and TI$ -- jiffy clock (1/60s ticks since power on).
 *   Map to platform_ticks_ms() and format as HH:MM:SS.
 */

#include "dialect.h"

static const DialectConfig c64_config = {
    DIALECT_COMMODORE,
    "Commodore BASIC v2",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY.", 10, 0, 0, 0,
    "C64B", DFLAG_C64B
};

void dialect_register_c64(void)
{
    dialect_register(&c64_config);
}
