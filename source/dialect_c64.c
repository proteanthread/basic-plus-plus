/*
 * dialect_c64.c -- Commodore BASIC v2 (C64/VIC-20/PET)
 *
 * Microsoft BASIC v2.0, licensed 1977, never updated for C64.
 * Despite the SID and VIC-II hardware, BASIC v2 has NO commands
 * for sound, sprites, or bitmap graphics -- all via PEEK/POKE.
 *
 * "READY." prompt (with period). 10-column zones. No ON ERROR,
 * no WHILE/WEND, no CLS (use PRINT CHR$(147)).
 *
 * TODO: VIC-II memory map ($D000-$D02E) via virtual PEEK/POKE
 * TODO: SID registers ($D400-$D418) for sound
 * TODO: RND(0) repeat-last-value behavior
 * TODO: TI/TI$ jiffy clock
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
