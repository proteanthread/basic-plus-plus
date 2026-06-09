/*
 * dialect_atari.c -- Atari BASIC (400/800/XL/XE)
 *
 * Shepardson Microsystems, 1979. NOT a Microsoft BASIC.
 * 8KB cartridge ROM, 6502 @ 1.79 MHz. Programs stored as
 * tokens (binary SAVE format). Infamously slow.
 *
 * Key differences: DIM required for strings, 10-column print
 * zones, TRAP for error handling, A$(5,10) substring syntax
 * instead of MID$. GRAPHICS 0-8 for display modes.
 *
 * TODO: GRAPHICS n (modes 0-8, up to 320x192)
 * TODO: SETCOLOR register, hue, luminance
 * TODO: PLOT/DRAWTO/POSITION/LOCATE
 * TODO: SOUND voice, freq, distortion, volume (POKEY)
 * TODO: A$(m,n) substring syntax
 */

#include "dialect.h"

static const DialectConfig atari_config = {
    DIALECT_ATARI_MS,
    "Atari BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    32767, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY",
    10,                     /* narrower zones than Microsoft */
    1, 0, 1,                /* TRAP maps to on_error */
    "ATRI", DFLAG_ATRI
};

void dialect_register_atari(void)
{
    dialect_register(&atari_config);
}
