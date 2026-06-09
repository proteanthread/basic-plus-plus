/*
 * =====================================================================
 * BASIC++ Dialect Profile — AppleSoft BASIC (ASFT)
 * =====================================================================
 *
 * MACHINE:   Apple II, II+, IIe, IIc, IIGS
 * AUTHOR:    Microsoft (Applesoft — licensed by Apple)
 * YEAR:      1977 (cassette), 1978 (Apple II+ ROM)
 * ROM SIZE:  12KB (at $D000-$FFFF, replacing Integer BASIC)
 * RAM:       48KB typical (Apple IIe: 64KB-128KB)
 * CPU:       MOS 6502 @ 1.023 MHz (65C02 in IIe/IIc)
 * DISPLAY:   Text: 40x24 (80x24 with 80-col card);
 *            Lo-Res: 40x48 (16 colors);
 *            Hi-Res: 280x192 (6 colors on NTSC);
 *            Double Hi-Res: 560x192 (IIe/IIc with 128KB)
 * STORAGE:   Floppy disk (Disk II), cassette
 * SOUND:     1-bit speaker toggle (same as Integer BASIC)
 *
 * HISTORICAL NOTES:
 *   Microsoft BASIC for the Apple II, replacing Wozniak's Integer
 *   BASIC. Named "Applesoft" by Apple. Full 9-digit floating
 *   point (FAC/ARG format), standard Microsoft BASIC feature set.
 *
 *   The famous ']' prompt is the AppleSoft cursor. Maximum line
 *   number is 63999. ONERR GOTO handles errors (not ON ERROR GOTO).
 *
 *   AppleSoft was the standard Apple II BASIC from 1978 through
 *   the end of the Apple II line in the early 1990s.
 *
 * DIALECT QUIRKS (compared to GW-BASIC):
 *   - THEN required after IF
 *   - LET optional
 *   - No WHILE/WEND (must use FOR/NEXT or GOTO loops)
 *   - No DO/LOOP
 *   - No SWAP (must use temp variable)
 *   - ONERR GOTO (not ON ERROR GOTO)
 *   - No EDIT command, no AUTO
 *   - Prompt: ']' (right bracket)
 *   - GR — switch to lo-res graphics (40x48, 16 colors)
 *   - HGR — switch to hi-res page 1 (280x192)
 *   - HGR2 — switch to hi-res page 2
 *   - HCOLOR=n — set hi-res plot color (0-7)
 *   - HPLOT x,y [TO x2,y2 [TO x3,y3 ...]] — hi-res line
 *   - COLOR=n — set lo-res color (0-15)
 *   - PLOT x,y — lo-res pixel
 *   - HLIN x1,x2 AT y — horizontal line (lo-res)
 *   - VLIN y1,y2 AT x — vertical line (lo-res)
 *   - SCRN(x,y) — read lo-res color
 *   - TEXT — return to text mode
 *   - PDL(n) — read game paddle (0-255)
 *   - CALL addr — 6502 machine language call
 *   - & (ampersand) — hook to machine language extension
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Full floating-point arithmetic
 *   [x] String variables and string functions
 *   [x] DATA/READ/RESTORE
 *   [x] DIM arrays
 *   [x] DEF FN
 *   [x] ON GOTO/GOSUB
 *   [x] ON ERROR GOTO (via ONERR mapping)
 *   [x] PRINT with zones
 *   [x] ']' prompt
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] GR (lo-res graphics 40x48)
 *       Shared with Integer BASIC. See dialect_aint.c for
 *       implementation plan.
 *   [ ] HGR / HGR2 (hi-res graphics 280x192)
 *       Fix: Map to gfxbuf with 280x192 resolution.
 *       Add KW_HGR, KW_HGR2 to lexer.h, tag DFLAG_ASFT.
 *   [ ] HCOLOR=n (set hi-res color 0-7)
 *       Apple II hi-res colors: 0=black1, 1=green, 2=violet,
 *       3=white1, 4=black2, 5=orange, 6=blue, 7=white2.
 *       NTSC artifact colors — the Apple II's iconic look.
 *   [ ] HPLOT x,y TO x2,y2 (hi-res line drawing with TO chaining)
 *       Fix: Implement Bresenham's line algorithm in gfxbuf.c.
 *       Support multiple TO segments per HPLOT statement.
 *   [ ] COLOR=n, PLOT, HLIN, VLIN, SCRN(x,y) (lo-res)
 *       Share implementation with dialect_aint.c.
 *   [ ] TEXT (return to text mode)
 *   [ ] PDL(n) (paddle read — map to 128 default or VDev input)
 *   [ ] CALL addr (6502 call — stub only)
 *   [ ] & (ampersand) hook (stub — could be extension point)
 *   [ ] ONERR GOTO vs ON ERROR GOTO syntax difference
 *       Currently mapped as ON ERROR GOTO. The original ONERR
 *       keyword (no space) should be accepted as an alias.
 *   [ ] PEEK/POKE memory map (Apple II hardware addresses)
 *       Key addresses: $C030 (speaker), $C000 (keyboard),
 *       $C010 (keyboard strobe), soft switches for graphics.
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit asft_config to change feature flags.
 *   2. For HGR/HPLOT: implement hi-res mode in gfxbuf.c
 *      (280x192, 7 NTSC artifact colors). Add Bresenham
 *      line drawing for HPLOT ... TO ... chains.
 *   3. Share lo-res (GR/PLOT/HLIN/VLIN) with dialect_aint.c.
 *   4. For ONERR alias: add "ONERR" as keyword alias in lexer.c,
 *      mapping to KW_ON + KW_ERROR (or a new KW_ONERR).
 *   5. Rebuild. Test with: DIALECT "ASFT"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig asft_config = {
    DIALECT_APPLESOFT,          /* id */
    "AppleSoft BASIC",          /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function: NO — uses FRE(0) */
    63999,  /* max_line_number */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    1,      /* has_extended_vars */
    0,      /* has_merge_chain: NO */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "]",        /* ready_prompt: the famous bracket */
    16,         /* print_zone_width */
    1,          /* has_on_error: YES (ONERR GOTO) */
    0,          /* has_cls: NO (use CALL -936) */
    0,          /* has_tron_troff: NO */
    "ASFT",     /* short_name */
    DFLAG_ASFT  /* dialect_flag */
};

void dialect_register_asft(void)
{
    dialect_register(&asft_config);
}
