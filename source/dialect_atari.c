/*
 * =====================================================================
 * BASIC++ Dialect Profile — Atari BASIC (ATRI)
 * =====================================================================
 *
 * MACHINE:   Atari 400, 800, 1200XL, 600XL, 800XL, 65XE, 130XE
 * AUTHOR:    Shepardson Microsystems (Paul Laughton, Kathleen O'Brien)
 * YEAR:      1979
 * ROM SIZE:  8KB cartridge (Rev. A/B/C)
 * RAM:       16KB (400), 48KB (800), 64KB (XL/XE)
 * CPU:       MOS 6502 @ 1.79 MHz (PAL: 1.77 MHz)
 * DISPLAY:   Text: 40x24; Graphics: 8 modes (GRAPHICS 0-8),
 *            up to 320x192 (4 colors) or 160x192 (16 colors)
 * SOUND:     POKEY chip — 4 independent voices, 8-bit resolution
 * STORAGE:   Cassette (410), floppy disk (810/1050)
 * CUSTOM HW: ANTIC (display list), GTIA (player/missile graphics),
 *            POKEY (sound + I/O + keyboard + serial)
 *
 * HISTORICAL NOTES:
 *   NOT a Microsoft BASIC. Shepardson Microsystems wrote Atari BASIC
 *   from scratch. It's unique among 8-bit BASICs: programs are
 *   tokenized on entry (stored as tokens, not text), which makes
 *   SAVE files binary (not ASCII listable).
 *
 *   Infamously slow — the tokenizer and runtime are poorly
 *   optimized. Nested FOR loops and string operations are
 *   especially painful. Despite this, Atari BASIC was the
 *   standard BASIC for all Atari 8-bit computers.
 *
 * DIALECT QUIRKS (compared to Microsoft BASIC):
 *   - DIM REQUIRED for strings: DIM A$(20) allocates 20 chars
 *   - No auto-extending strings — length set at DIM time
 *   - CLR clears variables (not CLS for screen clear)
 *   - PRINT has 10-column zones (not 14 like Microsoft)
 *   - TRAP line — error handler (instead of ON ERROR GOTO)
 *     TRAP sets a line to jump to on ANY error.
 *   - GRAPHICS n — switch display mode (0=text, 1-8=graphics)
 *   - SETCOLOR reg, hue, luminance — set palette register
 *   - COLOR n — set plot color index
 *   - PLOT x,y — set pixel in graphics mode
 *   - DRAWTO x,y — draw line from current position
 *   - POSITION x,y — move cursor without plotting
 *   - LOCATE x,y,c — read pixel color into variable c
 *   - SOUND voice, freq, distortion, volume — POKEY sound
 *   - XIO command, #channel, aux1, aux2, "device"
 *     (extended I/O — the universal device command)
 *   - ENTER / LIST to cassette or disk
 *   - Programs are tokenized on entry (binary SAVE format)
 *
 * WHAT'S IMPLEMENTED:
 *   [x] DIM for strings
 *   [x] CLR (clear variables)
 *   [x] 10-column PRINT zones
 *   [x] TRAP error handler
 *   [x] DATA/READ/RESTORE
 *   [x] Full floating-point
 *   [x] String functions (LEN, ASC, CHR$, VAL, STR$)
 *   [x] DEF FN
 *   [x] DIM arrays
 *   [x] "READY" prompt
 *   [x] TRON/TROFF (Atari has this)
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] GRAPHICS n (switch display mode 0-8)
 *       Atari modes:
 *         0: 40x24 text (no graphics window)
 *         1: 20x24 text, 4 colors, large characters
 *         2: 20x12 text, 4 colors, very large characters
 *         3: 40x24, 4 colors (lo-res, like Apple GR)
 *         4: 80x48, 2 colors
 *         5: 80x48, 4 colors
 *         6: 160x96, 2 colors
 *         7: 160x96, 4 colors
 *         8: 320x192, 1.5 colors (mono + background)
 *       Fix: Map each mode to appropriate gfxbuf resolution.
 *       Add KW_GRAPHICS to lexer.h, tag with DFLAG_ATRI.
 *       Mode number sets resolution and color count.
 *   [ ] SETCOLOR register, hue, luminance
 *       Atari uses 16 hues x 8 luminance = 128 colors.
 *       Fix: Map to gfxbuf palette system. SETCOLOR sets
 *       one of 5 color registers (0-4).
 *   [ ] COLOR n (set plot color register index, 0-3)
 *   [ ] PLOT x,y (set pixel at graphics cursor)
 *   [ ] DRAWTO x,y (line from current pos to x,y)
 *       Fix: Bresenham line algorithm in gfxbuf.c.
 *   [ ] POSITION x,y (move cursor without plotting)
 *   [ ] LOCATE x,y,c (read pixel color into variable c)
 *   [ ] SOUND voice, freq, distortion, volume
 *       The POKEY chip has 4 voices. Each takes:
 *         voice: 0-3
 *         freq: 0-255 (frequency divider)
 *         dist: 0, 2, 4, 6, 8, 10, 12, 14 (distortion)
 *         vol: 0-15
 *       Fix: Map voice 0 to PC speaker/BEEP. Store state
 *       for VDev-backed audio in future.
 *   [ ] XIO command (extended I/O — complex, low priority)
 *   [ ] ENTER (load ASCII listing from device)
 *   [ ] Tokenized SAVE format (binary, not ASCII)
 *       We always save as ASCII. Original Atari SAVE was binary.
 *   [ ] String DIM enforcement (error if string used without DIM)
 *   [ ] A$(start,end) substring syntax (not MID$/LEFT$/RIGHT$)
 *       Atari BASIC uses A$(5,10) instead of MID$(A$,5,6).
 *       Fix: Detect A$(m,n) in expression parser, translate
 *       to internal MID$ equivalent.
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit atari_config to change feature flags.
 *   2. For GRAPHICS modes: implement mode table in gfxbuf.c,
 *      add KW_GRAPHICS handler in parser.c.
 *   3. For A$(m,n) substring syntax: modify string expression
 *      parsing to detect two-argument subscript on string
 *      variables, translate to MID$ internally.
 *   4. For SOUND: add a POKEY state struct, map voice 0 to
 *      platform audio. Use VDev for audio device abstraction.
 *   5. Rebuild. Test with: DIALECT "ATRI"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig atari_config = {
    DIALECT_ATARI_MS,           /* id (historical name, not MS BASIC) */
    "Atari BASIC",              /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars: YES (but DIM required) */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function */
    32767,  /* max_line_number */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    1,      /* has_extended_vars */
    0,      /* has_merge_chain: NO */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "READY",    /* ready_prompt */
    10,         /* print_zone_width: 10 columns (not 14!) */
    1,          /* has_on_error: YES (TRAP) */
    0,          /* has_cls: NO (use PRINT CHR$(125)) */
    1,          /* has_tron_troff */
    "ATRI",     /* short_name */
    DFLAG_ATRI  /* dialect_flag */
};

void dialect_register_atari(void)
{
    dialect_register(&atari_config);
}
