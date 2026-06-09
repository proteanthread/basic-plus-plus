/*
 * dialect_asft.c -- AppleSoft BASIC (Apple II+/IIe/IIc)
 *
 * Microsoft BASIC for Apple II, 1978. 12KB ROM replacing
 * Wozniak's Integer BASIC. Full floating point, ']' prompt.
 * ONERR GOTO for error handling (not ON ERROR GOTO).
 * No WHILE/WEND -- use FOR/NEXT or GOTO loops.
 *
 * TODO: HGR/HGR2 hi-res 280x192 graphics
 * TODO: HPLOT x,y TO x2,y2 (Bresenham line)
 * TODO: HCOLOR= (NTSC artifact colors 0-7)
 * TODO: lo-res GR/PLOT/HLIN/VLIN (shared with Integer BASIC)
 * TODO: ONERR keyword alias
 */

#include "dialect.h"

static const DialectConfig asft_config = {
    DIALECT_APPLESOFT,
    "AppleSoft BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    63999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "]",                    /* the famous bracket prompt */
    16, 1, 0, 0,
    "ASFT", DFLAG_ASFT
};

void dialect_register_asft(void)
{
    dialect_register(&asft_config);
}
