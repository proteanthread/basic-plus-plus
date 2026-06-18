 // dialect_sinclair.c -- Sinclair BASIC (ZX Spectrum 48K, 1982)
 //
 // The built-in BASIC interpreter of the Sinclair ZX Spectrum,
 // developed by Nine Tiles Networks (Steve Vickers, John Grant).
 // Also known as "48 BASIC" or "Spectrum BASIC".
 //
 // Originally derived from the ZX80's 4K BASIC (integer only),
 // expanded to 8K BASIC for the ZX81 (adding floating point),
 // then to 16K ROM for the Spectrum (adding colour, sound,
 // graphics, and the UDG character set).
 //
 // KEY DIFFERENCES FROM MICROSOFT BASIC:
 //   - LET is REQUIRED (not optional)
 //   - GO TO / GO SUB (spaced) are canonical forms
 //   - LN instead of LOG for natural logarithm
 //   - No WHILE/WEND, no DO/LOOP, no ON ERROR GOTO
 //   - No ELSE keyword (IF...THEN only)
 //   - RANDOMIZE instead of RANDOMIZE/RND(-n)
 //   - DIM for strings sets fixed length: DIM A$(10) = 10 chars
 //   - Arrays are 1-based by default
 //   - PRINT zones are 16 columns wide (32-col screen)
 //   - Max line number: 9999
 //   - Ready prompt: "0 OK, line:col" (simplified to "0 OK")
 //
 // SPECTRUM-SPECIFIC KEYWORDS:
 //   - BORDER n (border colour 0-7)
 //   - INK n / PAPER n (foreground/background colour)
 //   - BRIGHT n / FLASH n / INVERSE n / OVER n
 //   - PLOT x,y / DRAW x,y[,angle]
 //   - BEEP duration, pitch
 //   - PAUSE n (1/50th second frames)
 //   - COPY (screen dump to printer)
 //   - IN addr / OUT addr, val (Z80 port I/O)
 //   - PEEK/POKE (Z80 memory access)
 //
 // NOT AVAILABLE IN SPECTRUM BASIC:
 //   - WHILE/WEND, DO/LOOP (no structured loops)
 //   - ON ERROR GOTO (no error trapping)
 //   - ELSE (IF...THEN only, no ELSE branch)
 //   - SELECT CASE
 //   - SUB/FUNCTION (DEF FN only, single-letter names)
 //   - TRON/TROFF
 //   - Multi-statement IF (only single line)

#include "dialect.h"

 // sinclair_apply - Apply Sinclair BASIC configuration.
 //
 // Sinclair BASIC has a unique personality:
 //   - LET mandatory (has_let_optional = 0)
 //   - LN aliases LOG (handled in keyword table: LN -> KW_LOG_FUNC)
 //   - No WHILE/WEND, no DO/LOOP, no ON ERROR
 //   - MERGE supported (for loading code from tape)
 //   - CLS available (has_cls = 1)
 //   - No TRON/TROFF
 //   - BORDER/INK/PAPER/BRIGHT/FLASH/INVERSE/OVER are display attrs
 //   - PLOT maps to PSET, DRAW draws relative lines
 //   - BEEP for sound (2 params: duration, pitch)
 //   - PAUSE halts for n frames (1/50th sec)
 //   - IN/OUT for Z80 port I/O
 //   - "0 OK" prompt, 16-column zones, max line 9999
static void sinclair_apply(void)
{
 // Sinclair's unique keywords (BORDER, INK, PAPER, BRIGHT,
  // FLASH, INVERSE, OVER) are all tagged DFLAG_SINC in the
  // keyword table. LN is aliased to LOG in the lexer (both
  // map to KW_LOG_FUNC). PLOT maps to KW_PSET, and IN maps
  // to KW_INP. The MERGE command is supported for loading
  // program fragments (tagged DFLAG_SINC). 
}

static const DialectConfig sinclair_config = {
    DIALECT_SINCLAIR,
    "Sinclair BASIC (ZX Spectrum)",
    ':', 1, 0, 1, 1, 0, 0, 1, 1, 0,
    9999, 0, 1, 0, 0, 1, 1,
    1, 1, 1,
    "0 OK", 16, 0, 1, 0,
    "SINC", DFLAG_SINC, sinclair_apply
};

void dialect_register_sinclair(void)
{
    dialect_register(&sinclair_config);
}
