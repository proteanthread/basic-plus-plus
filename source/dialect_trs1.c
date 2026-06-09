/*
 * =====================================================================
 * BASIC++ Dialect Profile — TRS-80 Level I BASIC (TRS1)
 * =====================================================================
 *
 * MACHINE:   TRS-80 Model I (Level I ROM)
 * AUTHOR:    Steve Leininger (based on Li-Chen Wang's Tiny BASIC)
 * YEAR:      1977
 * ROM SIZE:  4KB
 * RAM:       4KB-16KB (4KB standard, 16KB with expansion)
 * CPU:       Zilog Z80 @ 1.774 MHz
 * DISPLAY:   64x16 text, 128x48 block graphics (SET/RESET/POINT)
 * STORAGE:   Cassette tape
 *
 * HISTORICAL NOTES:
 *   Radio Shack's entry-level BASIC for the TRS-80 Model I —
 *   the first mass-market home computer (along with Apple II
 *   and Commodore PET, all released in 1977). Level I was
 *   extremely limited: integer-only, basic string support
 *   (only A$ was reliable), no string arrays, 2-byte integers.
 *
 *   Despite its limitations, Level I BASIC was how millions of
 *   people first learned to program. The TRS-80 was affectionately
 *   (and not-so-affectionately) known as the "Trash-80."
 *
 * DIALECT QUIRKS (compared to Microsoft BASIC):
 *   - Integer-only arithmetic (2-byte, -32768 to 32767)
 *   - Very basic string support (A$ works, but arrays don't)
 *   - No string arrays
 *   - No DEF FN user functions
 *   - No WHILE/WEND, no DO/LOOP
 *   - No MERGE/CHAIN
 *   - No TRON/TROFF, no ON ERROR
 *   - CLS clears 64x16 screen
 *   - SET(x,y)/RESET(x,y)/POINT(x,y) for block graphics
 *   - PRINT zone width: 16 columns
 *   - Prompt: "READY"
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Colon statement separator
 *   [x] THEN keyword required
 *   [x] Integer arithmetic
 *   [x] Basic string variable support
 *   [x] DATA/READ/RESTORE
 *   [x] DIM arrays
 *   [x] CLS
 *   [x] PRINT zone width 16
 *   [x] "READY" prompt
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] SET(x,y) / RESET(x,y) / POINT(x,y) block graphics
 *       The TRS-80 displayed 128x48 "pixels" using 2x3 blocks
 *       within each 64x16 character cell. Each SET/RESET toggles
 *       one block. POINT returns whether a block is set.
 *       Fix: Map to gfxbuf with appropriate resolution, render
 *       via Unicode half-block characters (▀▄█ etc.).
 *   [ ] MEM function (free memory — currently mapped to SIZE)
 *   [ ] Single-string limitation (only A$ fully reliable)
 *   [ ] 2-byte integer overflow behavior (wraps at 32767)
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit trs1_config below to change feature flags.
 *   2. To add SET/RESET/POINT:
 *      a. Add KW_SET_GFX, KW_RESET_GFX, KW_POINT_GFX to lexer.h
 *      b. Tag them with DFLAG_TRS1 in lexer.c keyword table
 *      c. Add handlers in parser.c that call gfxbuf_* functions
 *      d. Set has_block_graphics=1 here (add field to DialectConfig)
 *   3. Rebuild. Test with: DIALECT "TRS1"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig trs1_config = {
    DIALECT_TRS80_L1,           /* id */
    "TRS-80 Level I BASIC",     /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword: YES */
    1,      /* has_let_optional: YES */
    1,      /* has_for_next: YES */
    1,      /* has_string_vars: YES (A$ only, limited) */
    0,      /* has_print_hash: NO */
    0,      /* has_array_at: NO — uses DIM A(n) */
    1,      /* has_rnd_function: YES */
    1,      /* has_abs_function: YES */
    1,      /* has_size_function: YES (as MEM) */
    32767,  /* max_line_number */
    0,      /* not_eq_is_hash: NO — uses <> */
    1,      /* has_data_read: YES */
    0,      /* has_while_wend: NO */
    0,      /* has_do_loop: NO */
    0,      /* has_extended_vars: NO */
    0,      /* has_merge_chain: NO */
    0,      /* has_float: NO — integer only */
    1,      /* has_dim_arrays: YES */
    1,      /* has_string_functions: YES (limited) */
    /* Phase 8: Dialect personality */
    "READY",    /* ready_prompt */
    16,         /* print_zone_width */
    0,          /* has_on_error: NO */
    1,          /* has_cls: YES */
    0,          /* has_tron_troff: NO */
    "TRS1",     /* short_name */
    DFLAG_TRS1  /* dialect_flag */
};

void dialect_register_trs1(void)
{
    dialect_register(&trs1_config);
}
