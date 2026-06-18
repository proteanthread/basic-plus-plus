 // dialect_mbasic.c -- Microsoft MBASIC / BASIC-80 (CP/M, 1978-1981)
 //
 // The CP/M version of Microsoft BASIC, also known as BASIC-80
 // version 5.x.  Direct descendant of Altair BASIC and ancestor
 // of GW-BASIC.  Ran on Intel 8080/Z80 systems under CP/M.
 //
 // MBASIC is a pure text/computation BASIC with NO hardware-
 // specific features: no graphics, no sound, no CLS, no LOCATE,
 // no color, no joystick, no screen modes.  All I/O goes through
 // the CP/M console and line printer interfaces.
 //
 // "Ok" prompt (same as GW-BASIC -- GW inherited it from MBASIC).
 // 14-column PRINT zones.  Max line 65529.
 //
 // Key features:
 //   - WHILE/WEND (v5.0+)
 //   - ON ERROR GOTO / RESUME
 //   - CHAIN / MERGE / COMMON (program overlay system)
 //   - DEFINT/DEFSNG/DEFDBL/DEFSTR type declarations
 //   - Variable names up to 40 characters
 //   - Arrays up to 7 dimensions
 //   - Data types: integer, single, double, string
 //   - Sequential and random-access file I/O
 //   - TRON/TROFF trace
 //   - DEF FN user functions
 //   - PEEK/POKE/INP/OUT (8080 memory/port access)
 //   - DEF USR / USR() machine language calls
 //
 // NOT available in MBASIC:
 //   - Graphics (PSET, LINE, CIRCLE, DRAW, PAINT, SCREEN, COLOR)
 //   - Sound (PLAY, BEEP, SOUND)
 //   - CLS, LOCATE, CSRLIN, POS (terminal-dependent)
 //   - KEY (function key programming)

#include "dialect.h"

 // mbasic_apply - Apply MBASIC/BASIC-80 configuration.
 //
 // MBASIC is the CP/M text-only Microsoft BASIC:
 //   - Full WHILE/WEND support (has_while_wend = 1)
 //   - ON ERROR GOTO / RESUME (has_on_error = 1)
 //   - CHAIN / MERGE with COMMON (has_merge_chain = 1)
 //   - DEFINT/DEFSNG/DEFDBL/DEFSTR (full type system)
 //   - TRON/TROFF (has_tron_troff = 1)
 //   - No graphics, no sound, no CLS (terminal-independent)
 //   - 14-column print zones (Microsoft standard)
 //   - "Ok" prompt
 //
 // This is the ancestor of GW-BASIC and QBasic, with all the
 // computation and I/O features but none of the display features.
static void mbasic_apply(void)
{
 // MBASIC has all the Microsoft BASIC computation features.
  // Its keywords are included via DFLAG_MSBASIC and DFLAG_MBAS.
  // CHAIN and MERGE are now correctly tagged with DFLAG_MBAS
  // in the keyword table. TRON/TROFF use the same trace
  // mechanism as GW-BASIC. 
}

static const DialectConfig mbasic_config = {
    DIALECT_MBASIC,
    "Microsoft MBASIC (CP/M)",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 0, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 0, 1,
    "MBAS", DFLAG_MBAS, mbasic_apply
};

void dialect_register_mbasic(void)
{
    dialect_register(&mbasic_config);
}
