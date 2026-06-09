/*
 * =====================================================================
 * BASIC++ Dialect Profile — Color Computer BASIC (COCO)
 * =====================================================================
 *
 * MACHINE:   TRS-80 Color Computer (CoCo 1, CoCo 2, CoCo 3)
 * AUTHOR:    Microsoft (Color BASIC + Extended Color BASIC)
 * YEAR:      1980 (CoCo 1), 1983 (CoCo 2), 1986 (CoCo 3)
 * ROM SIZE:  16KB (8KB Color BASIC + 8KB Extended Color BASIC)
 * RAM:       4KB-512KB (CoCo 3 could have 512KB)
 * CPU:       Motorola 6809 @ 0.895 MHz (CoCo 1/2),
 *            0.895/1.79 MHz switchable (CoCo 3)
 * DISPLAY:   Text: 32x16 (CoCo 1/2), 40x24/80x24 (CoCo 3);
 *            Graphics: 256x192 (4 colors), 320x200 (CoCo 3)
 * SOUND:     1-bit DAC (simple beeps), multi-voice PLAY
 * STORAGE:   Cassette, floppy disk
 *
 * HISTORICAL NOTES:
 *   The TRS-80 Color Computer ("CoCo") used the Motorola 6809
 *   CPU — far more powerful than the Z80 or 6502, with proper
 *   16-bit operations and a clean instruction set. Despite this,
 *   the BASIC was fairly standard Microsoft.
 *
 *   The CoCo has dedicated graphics commands unlike the C64,
 *   making it much easier to program for games. The PLAY
 *   command uses a music macro language similar to GW-BASIC.
 *
 *   "OK" prompt (uppercase, not "Ok" like GW-BASIC).
 *
 * DIALECT QUIRKS (compared to GW-BASIC):
 *   - "OK" prompt (uppercase)
 *   - No WHILE/WEND (must use FOR/NEXT or GOTO loops)
 *   - No DO/LOOP
 *   - ON ERROR GOTO supported
 *   - CLS with optional color argument: CLS n (0-8)
 *   - TRON/TROFF
 *   - COLOR fg, bg (text and background colors)
 *   - SCREEN mode, colorset
 *   - PSET(x,y,c) / PRESET(x,y) (pixel plot/clear)
 *   - LINE(x1,y1)-(x2,y2), color [,B[F]] (line/box/filled box)
 *   - CIRCLE(x,y), radius [,color] (circle drawing)
 *   - PAINT(x,y) [,color [,border]] (flood fill)
 *   - PLAY music macro string
 *   - SOUND freq, duration
 *   - INKEY$ (read single keypress)
 *   - JOYSTK(n) — joystick position (0-63)
 *   - No LOCATE (no direct cursor positioning like GW-BASIC)
 *   - MOTOR ON/OFF (cassette motor control)
 *   - CLOAD/CSAVE (cassette programs)
 *   - 16-column PRINT zones
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Core Microsoft BASIC
 *   [x] DATA/READ/RESTORE
 *   [x] DIM arrays, DEF FN
 *   [x] ON ERROR GOTO / RESUME
 *   [x] CLS
 *   [x] TRON/TROFF
 *   [x] COLOR
 *   [x] SCREEN
 *   [x] PLAY
 *   [x] SOUND
 *   [x] INKEY$
 *   [x] PSET
 *   [x] LINE
 *   [x] PAINT
 *   [x] "OK" prompt
 *   [x] 16-column PRINT zones
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] CIRCLE(x,y), radius [,color [,hw [,start,end]]]
 *       CoCo CIRCLE has hardware ratio and arc parameters.
 *       Fix: Implement in gfxbuf.c, wire to parser.c.
 *   [ ] LINE ... ,BF (filled box mode)
 *       LINE with ,B draws a box, ,BF fills it.
 *       Fix: Detect B/BF suffix in LINE parser, call
 *       gfxbuf_rect() or gfxbuf_filled_rect().
 *   [ ] PMODE n, start_page (set graphics page mode)
 *       CoCo graphics modes (0-4) set resolution and colors.
 *       Fix: Map to gfxbuf mode table.
 *   [ ] PCLS [color] (clear graphics page)
 *   [ ] PCOPY src TO dest (copy graphics pages)
 *   [ ] JOYSTK(n) — joystick (map to VDev input device)
 *   [ ] MOTOR ON/OFF (cassette — stub only)
 *   [ ] CLOAD/CSAVE (map to LOAD/SAVE aliases)
 *   [ ] CoCo 3 enhanced modes (HSCREEN, HCOLOR, HPRINT...)
 *       The CoCo 3 GIME chip added 320x200 and 640x200 modes.
 *       This is a separate dialect variant (COCO3).
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit coco_config to change feature flags.
 *   2. For CIRCLE: implement midpoint circle algorithm in
 *      gfxbuf.c with aspect ratio and arc support.
 *   3. For LINE ,BF: detect B/BF flags in parser.c LINE
 *      handler, call filled rect function.
 *   4. For PMODE: add mode table (0-4) with resolution
 *      and color mapping.
 *   5. For CoCo 3: consider creating dialect_coco3.c as a
 *      separate dialect that extends COCO with HSCREEN etc.
 *   6. Rebuild. Test with: DIALECT "COCO"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig coco_config = {
    DIALECT_COCO,               /* id */
    "Color Computer BASIC",     /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function */
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
    "OK",       /* ready_prompt: uppercase (not "Ok") */
    16,         /* print_zone_width */
    1,          /* has_on_error */
    1,          /* has_cls */
    1,          /* has_tron_troff */
    "COCO",     /* short_name */
    DFLAG_COCO  /* dialect_flag */
};

void dialect_register_coco(void)
{
    dialect_register(&coco_config);
}
