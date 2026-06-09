/*
 * =====================================================================
 * BASIC++ Dialect Profile — GW-BASIC (GWBS)
 * =====================================================================
 *
 * MACHINE:   IBM PC, XT, AT, and 100% compatibles
 * AUTHOR:    Microsoft (Greg Whitten — "Gee Whiz")
 * YEAR:      1983 (BASICA 1981 for IBM, GW-BASIC 1983 for clones)
 * ROM SIZE:  N/A — loaded from disk (~75KB .EXE)
 * RAM:       60KB BASIC workspace within 640KB DOS
 * CPU:       Intel 8088/8086/80286 and above
 * DISPLAY:   Text: 80x25 or 40x25; Graphics: CGA/EGA/VGA modes
 * STORAGE:   Floppy disk, hard disk
 * SOUND:     PC speaker (BEEP, SOUND, PLAY)
 *
 * HISTORICAL NOTES:
 *   THE workhorse BASIC of the IBM PC era. GW-BASIC shipped with
 *   MS-DOS from version 1.0 through 5.0 (1981-1991). Nearly every
 *   BASIC program written for DOS in the 1980s targeted GW-BASIC.
 *
 *   GW-BASIC is the primary dialect for David Ahl's "BASIC Computer
 *   Games" on IBM PC. BASIC++ achieves 104/104 BCG game compilation
 *   in GWBS dialect mode.
 *
 *   The "Ok" prompt (lowercase 'k') is distinctive. Max line number
 *   is 65529 (not 65535 — 65530-65535 reserved internally).
 *
 * DIALECT QUIRKS:
 *   - WHILE/WEND loops (but no DO/LOOP — that's QBasic)
 *   - ON ERROR GOTO / RESUME / RESUME NEXT
 *   - SWAP A, B (exchange variables)
 *   - LINE INPUT for reading full lines
 *   - PRINT USING for formatted output
 *   - SOUND freq, duration (PC speaker)
 *   - PLAY music macro language
 *   - DRAW graphics macro language
 *   - SCREEN mode switching (0-13)
 *   - COLOR fg, bg (text colors)
 *   - DEF SEG / PEEK / POKE (memory access)
 *   - FIELD / GET / PUT / LSET / RSET (random access files)
 *   - KEY ON/OFF/LIST (function key display)
 *
 * WHAT'S IMPLEMENTED:
 *   [x] All core statements (PRINT, LET, IF/THEN/ELSE, FOR/NEXT...)
 *   [x] WHILE/WEND
 *   [x] ON ERROR GOTO / RESUME
 *   [x] SWAP, RANDOMIZE, BEEP
 *   [x] SOUND (PC speaker on Windows, bell on other platforms)
 *   [x] PLAY music macro language
 *   [x] DRAW graphics macro language (text-mode rendering)
 *   [x] SCREEN mode switching
 *   [x] COLOR fg, bg
 *   [x] WIDTH columns
 *   [x] DEF SEG / PEEK / POKE (virtual memory segment)
 *   [x] FIELD / GET / PUT / LSET / RSET (random access files)
 *   [x] PRINT USING
 *   [x] LINE INPUT
 *   [x] FILES / DIR
 *   [x] All math functions (SIN, COS, TAN, ATN, SQR, LOG, EXP...)
 *   [x] All string functions (LEFT$, RIGHT$, MID$, INSTR, ...)
 *   [x] CVI/CVS/CVD, MKI$/MKS$/MKD$ (binary conversion)
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] CIRCLE (x,y), radius [, color [, start, end [, aspect]]]
 *       Full ellipse/arc drawing. Currently uses gfxbuf but the
 *       parameter parsing for start/end angles and aspect ratio
 *       is not yet complete.
 *       Fix: Complete angle parsing in parser.c (SCREEN/graphics
 *       section), call gfxbuf_circle() with proper parameters.
 *   [ ] PAINT (x,y) [, fill_color [, border_color]]
 *       Flood fill. Requires a stack-based fill algorithm.
 *       Fix: Implement flood fill in gfxbuf.c, wire to parser.
 *   [ ] WINDOW / VIEW (viewport mapping)
 *   [ ] PCOPY (page copy between video pages)
 *   [ ] STRIG / STICK (joystick — may not be relevant)
 *   [ ] PEN (light pen — definitely not relevant)
 *   [ ] COM (serial port trapping via ON COM GOSUB)
 *   [ ] Full LOCATE row, col, cursor (cursor visibility toggle)
 *   [ ] KEY(n) ON/OFF/STOP + ON KEY(n) GOSUB (function key traps)
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit gwbs_config to change feature flags.
 *   2. For CIRCLE/PAINT: implement in gfxbuf.c, add parser
 *      handlers under the KW_CIRCLE / KW_PAINT cases.
 *   3. For joystick/COM: add VDev-backed devices (see vdev.h).
 *   4. Rebuild. Test with: DIALECT "GWBS"
 *   5. Validate with BCG test suite (104/104 target).
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig gwbs_config = {
    DIALECT_GW_BASIC,           /* id */
    "GW-BASIC",                 /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash: NO — GW uses PRINT USING instead */
    0,      /* has_array_at: NO — uses DIM */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function: NO — uses FRE(0) instead */
    65529,  /* max_line_number: 65530-65535 reserved */
    0,      /* not_eq_is_hash: NO — uses <> */
    1,      /* has_data_read */
    1,      /* has_while_wend */
    1,      /* has_do_loop: YES (BASIC++ extension for compat) */
    1,      /* has_extended_vars */
    1,      /* has_merge_chain */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    /* Phase 8: Dialect personality */
    "Ok",       /* ready_prompt: lowercase 'k' is authentic */
    14,         /* print_zone_width: 14 columns */
    1,          /* has_on_error */
    1,          /* has_cls */
    1,          /* has_tron_troff */
    "GWBS",     /* short_name */
    DFLAG_GWBS  /* dialect_flag */
};

void dialect_register_gwbs(void)
{
    dialect_register(&gwbs_config);
}
