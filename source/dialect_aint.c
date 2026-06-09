/*
 * =====================================================================
 * BASIC++ Dialect Profile — Apple II Integer BASIC (AINT)
 * =====================================================================
 *
 * MACHINE:   Apple II (original, 1977)
 * AUTHOR:    Steve Wozniak
 * YEAR:      1976-1977
 * ROM SIZE:  8KB (at $E000-$FFFF)
 * RAM:       4KB-48KB
 * CPU:       MOS 6502 @ 1.023 MHz
 * DISPLAY:   Text: 40x24; Lo-Res: 40x48 (16 colors);
 *            Hi-Res: 280x192 (6 colors, limited)
 * STORAGE:   Cassette, floppy disk (Disk II, 1978)
 * SOUND:     1-bit speaker toggle (click at $C030)
 *
 * HISTORICAL NOTES:
 *   Written entirely by Steve Wozniak in 6502 machine code —
 *   assembled BY HAND on paper, then typed in byte by byte.
 *   No assembler was used. This is one of the most remarkable
 *   programming achievements in computing history.
 *
 *   Integer BASIC is fast (16-bit integer math, no slow float)
 *   but limited: no floating point, no string arrays, mandatory
 *   LET. The prompt is just '>' with no "READY" message.
 *
 *   Apple replaced it with Microsoft's AppleSoft BASIC in 1978,
 *   but Integer BASIC remained available on the Apple II and
 *   was preferred for games (faster integer math).
 *
 * DIALECT QUIRKS:
 *   - LET is MANDATORY: must write LET A=5
 *   - Integer only: 16-bit signed (-32767 to 32767)
 *   - NO floating point at all
 *   - Strings: A$ works, but no string arrays
 *   - NO DATA/READ/RESTORE
 *   - NO WHILE/WEND, NO DO/LOOP
 *   - NO ON ERROR GOTO
 *   - NO extended variable names (A-Z only)
 *   - Prompt: '>' (just the chevron, very minimal)
 *   - GR — switch to lo-res graphics (40x48, 16 colors)
 *   - COLOR=n — set lo-res plot color (0-15)
 *   - PLOT x,y — set a lo-res pixel
 *   - HLIN x1,x2 AT y — horizontal line
 *   - VLIN y1,y2 AT x — vertical line
 *   - SCRN(x,y) — read color at lo-res position
 *   - TEXT — return to text mode
 *   - DSP — display variable on next change (debug)
 *   - CALL addr — call machine language subroutine
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Mandatory LET
 *   [x] Integer-only arithmetic
 *   [x] IF/THEN
 *   [x] FOR/NEXT
 *   [x] GOSUB/RETURN
 *   [x] PRINT with zones
 *   [x] Basic string support
 *   [x] '>' prompt
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] GR (switch to 40x48 lo-res graphics mode)
 *       Fix: Map to gfxbuf with 40x48 resolution. Use
 *       Unicode half-block characters for rendering.
 *       Add KW_GR to lexer.h, tag with DFLAG_AINT.
 *   [ ] COLOR=n (set lo-res color)
 *       Fix: Map to gfxbuf_set_color(). Note the '='
 *       syntax is unique — parsed as assignment-like.
 *   [ ] PLOT x,y (set lo-res pixel)
 *       Fix: Map to gfxbuf_pset(x, y, current_color).
 *   [ ] HLIN x1,x2 AT y (horizontal line)
 *   [ ] VLIN y1,y2 AT x (vertical line)
 *       Fix: Loop calling gfxbuf_pset() for each pixel.
 *   [ ] SCRN(x,y) (read pixel color)
 *       Fix: Map to gfxbuf_point(x, y).
 *   [ ] TEXT (return to text mode)
 *       Fix: Map to gfxbuf_close() or equivalent.
 *   [ ] DSP variable (debug display on change)
 *   [ ] CALL addr (6502 machine language call — stub only)
 *   [ ] MOD operator (Integer BASIC had MOD, not all MS BASICs do)
 *   [ ] String handling: DIM A$(length) required for strings
 *       (like Atari BASIC, not auto-allocated like MS BASIC)
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit aint_config to change feature flags.
 *   2. For lo-res graphics (GR/PLOT/HLIN/VLIN/SCRN/TEXT):
 *      a. Add KW_GR, KW_PLOT_LR, KW_HLIN, KW_VLIN, KW_SCRN_FUNC
 *         to lexer.h keyword enum.
 *      b. Tag with DFLAG_AINT in lexer.c keyword table.
 *      c. Add handlers in parser.c graphics section.
 *      d. Use gfxbuf 40x48 mode for rendering.
 *   3. Share PLOT/HLIN/VLIN implementation with AppleSoft (ASFT).
 *   4. Rebuild. Test with: DIALECT "AINT"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig aint_config = {
    DIALECT_APPLE_INT,              /* id */
    "Apple II Integer BASIC",       /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    0,      /* has_let_optional: NO — LET is MANDATORY */
    1,      /* has_for_next */
    0,      /* has_string_vars: NO (very limited) */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function */
    32767,  /* max_line_number */
    0,      /* not_eq_is_hash */
    0,      /* has_data_read: NO */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    0,      /* has_extended_vars: NO — A-Z only */
    0,      /* has_merge_chain: NO */
    0,      /* has_float: NO — integer only */
    1,      /* has_dim_arrays */
    0,      /* has_string_functions: NO (very limited) */
    ">",        /* ready_prompt: just chevron */
    16,         /* print_zone_width */
    0,          /* has_on_error: NO */
    0,          /* has_cls: NO (use CALL -936) */
    0,          /* has_tron_troff: NO */
    "AINT",     /* short_name */
    DFLAG_AINT  /* dialect_flag */
};

void dialect_register_aint(void)
{
    dialect_register(&aint_config);
}
