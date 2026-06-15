/*
 * dialect_mbasic.c -- Microsoft MBASIC / BASIC-80 (CP/M, 1978-1981)
 *
 * The CP/M version of Microsoft BASIC, also known as BASIC-80
 * version 5.x.  Direct descendant of Altair BASIC and ancestor
 * of GW-BASIC.  Ran on Intel 8080/Z80 systems under CP/M.
 *
 * MBASIC is a pure text/computation BASIC with NO hardware-
 * specific features: no graphics, no sound, no CLS, no LOCATE,
 * no color, no joystick, no screen modes.  All I/O goes through
 * the CP/M console and line printer interfaces.
 *
 * "Ok" prompt (same as GW-BASIC -- GW inherited it from MBASIC).
 * 14-column PRINT zones.  Max line 65529.
 *
 * Key features:
 *   - WHILE/WEND (v5.0+)
 *   - ON ERROR GOTO / RESUME
 *   - CHAIN / MERGE / COMMON (program overlay system)
 *   - DEFINT/DEFSNG/DEFDBL/DEFSTR type declarations
 *   - Variable names up to 40 characters
 *   - Arrays up to 7 dimensions
 *   - Data types: integer, single, double, string
 *   - Sequential and random-access file I/O
 *   - TRON/TROFF trace
 *   - DEF FN user functions
 *   - PEEK/POKE/INP/OUT (8080 memory/port access)
 *   - DEF USR / USR() machine language calls
 *
 * NOT available in MBASIC:
 *   - Graphics (PSET, LINE, CIRCLE, DRAW, PAINT, SCREEN, COLOR)
 *   - Sound (PLAY, BEEP, SOUND)
 *   - CLS, LOCATE, CSRLIN, POS (terminal-dependent)
 *   - KEY (function key programming)
 */

#include "dialect.h"

static const DialectConfig mbasic_config = {
    DIALECT_MBASIC,
    "Microsoft MBASIC (CP/M)",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 0, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 0, 1,
    "MBAS", DFLAG_MBAS, 0
};

void dialect_register_mbasic(void)
{
    dialect_register(&mbasic_config);
}
