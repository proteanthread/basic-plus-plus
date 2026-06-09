/*
 * =====================================================================
 * BASIC++ Dialect Profile — TRS-80 Level II BASIC (TRS2)
 * =====================================================================
 *
 * MACHINE:   TRS-80 Model I (Level II ROM), Model III, Model 4
 * AUTHOR:    Microsoft (licensed by Tandy/Radio Shack)
 * YEAR:      1978
 * ROM SIZE:  12KB (replacing 4KB Level I ROM)
 * RAM:       16KB-48KB
 * CPU:       Zilog Z80 @ 1.774 MHz (Model I), 2.03 MHz (Model III)
 * DISPLAY:   64x16 text, 128x48 block graphics
 * STORAGE:   Cassette, floppy disk (Model III/4)
 *
 * HISTORICAL NOTES:
 *   Full Microsoft BASIC. A massive upgrade over Level I: floating
 *   point (single and double precision), full string support with
 *   arrays, multi-dimensional DIM, DEF FN, ON GOTO/GOSUB, DATA/READ,
 *   multi-statement lines with ':' separator. This was the standard
 *   BASIC for serious TRS-80 development and the platform many of
 *   David Ahl's BASIC Computer Games ran on.
 *
 * DIALECT QUIRKS:
 *   - ON ERROR GOTO supported
 *   - WHILE/WEND supported (unusual for 1978)
 *   - TRON/TROFF for trace debugging
 *   - AUTO for auto-numbering
 *   - WIDTH for screen width control
 *   - EDIT for line editing
 *   - No DO/LOOP (WHILE/WEND only)
 *   - 14-column PRINT zones
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Full floating-point arithmetic
 *   [x] String variables and arrays
 *   [x] DATA/READ/RESTORE
 *   [x] DIM multi-dimensional arrays
 *   [x] DEF FN user functions
 *   [x] ON ERROR GOTO / RESUME
 *   [x] WHILE/WEND
 *   [x] TRON/TROFF
 *   [x] CLS
 *   [x] AUTO
 *   [x] SWAP, EDIT, WIDTH
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] SET(x,y)/RESET(x,y)/POINT(x,y) block graphics
 *   [ ] CLOAD/CSAVE (cassette load/save — could map to LOAD/SAVE)
 *   [ ] USR() machine language call (stubbed)
 *   [ ] VARPTR() — returns memory address of variable
 *   [ ] Double-precision (# suffix) vs single-precision (! suffix)
 *       Level II distinguished A! (single) from A# (double).
 *       Fix: Add type suffix parsing to lexer, store precision
 *       metadata in the variable table.
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit trs2_config to change feature flags.
 *   2. For SET/RESET/POINT, share implementation with TRS1.
 *   3. For CLOAD/CSAVE, alias to LOAD/SAVE in parser.c.
 *   4. Rebuild. Test with: DIALECT "TRS2"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig trs2_config = {
    DIALECT_TRS80_L2,           /* id */
    "TRS-80 Level II BASIC",    /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    1,      /* has_size_function */
    32767,  /* max_line_number */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    1,      /* has_while_wend */
    0,      /* has_do_loop */
    1,      /* has_extended_vars */
    0,      /* has_merge_chain */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "READY",    /* ready_prompt */
    14,         /* print_zone_width */
    1,          /* has_on_error */
    1,          /* has_cls */
    1,          /* has_tron_troff */
    "TRS2",     /* short_name */
    DFLAG_TRS2  /* dialect_flag */
};

void dialect_register_trs2(void)
{
    dialect_register(&trs2_config);
}
