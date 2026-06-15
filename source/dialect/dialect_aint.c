/*
 * dialect_aint.c -- Apple II Integer BASIC (Wozniak, 1977)
 *
 * Hand-assembled in 6502 without an assembler. 8KB ROM. Fast
 * (integer-only), quirky, '>' prompt. LET mandatory.
 * No floats, no DATA/READ -- not even Microsoft BASIC.
 *
 * The interesting gaps are the graphics commands:
 *   GR switches to lo-res (40x48, 16 colors, bottom 4 lines text).
 *   COLOR=n sets the plot color (0-15, mapped to Apple NTSC palette).
 *   PLOT x,y sets a single lo-res pixel.
 *   HLIN x1,x2 AT y -- horizontal line fill.
 *   VLIN y1,y2 AT x -- vertical line fill.
 *   SCRN(x,y) returns the color at a pixel.
 *
 *   All of these map cleanly to gfxbuf.c. GR initializes a 40x48
 *   buffer; PLOT/HLIN/VLIN are just pixel writes; SCRN reads back.
 *   The parser needs KW_GR, KW_PLOT, KW_HLIN, KW_VLIN, KW_SCRN
 *   in lexer.h with DFLAG_AINT|DFLAG_ASFT.
 *
 * MOD operator: would be a simple case in parse_expression, emit
 * fmod() for floats or % for ints.
 */

#include "dialect.h"

/*
 * aint_apply - Apply Apple II Integer BASIC configuration.
 *
 * Integer BASIC is unique among implemented dialects:
 *   - Integer-only: no floating point (has_float = 0)
 *   - No string variables (has_string_vars = 0)
 *   - No DATA/READ (has_data_read = 0)
 *   - LET mandatory (has_let_optional = 0)
 *   - No named/extended variables (has_extended_vars = 0)
 *   - No WHILE/WEND, no DO/LOOP
 *   - No ON ERROR, no MERGE/CHAIN
 *   - No DIM arrays (has_dim_arrays = 0)
 *   - No string functions (has_string_functions = 0)
 *   - '>' prompt (Wozniak's choice)
 *
 * Integer-only restriction is enforced via has_float gating
 * in the parser: float literals are silently truncated to
 * integers rather than raising errors.
 */
static void aint_apply(void)
{
 /* Integer-only mode is handled by has_float = 0 and the
  * parser's dialect_check_feature() gating. Float literals
  * are truncated (not rejected) per user specification.
  * CALL is available (DFLAG_AINT in keyword table) for
  * machine language subroutine calls. PEEK/POKE are also
  * available for memory access. */
}

static const DialectConfig aint_config = {
    DIALECT_APPLE_INT,
    "Apple II Integer BASIC",
    ':', 1,
    0,                      /* LET mandatory */
    1, 0, 0, 0, 1, 1, 0,
    32767, 0,
    0,                      /* no DATA/READ */
    0, 0, 0, 0, 0, 1, 0,
    ">", 16, 0, 0, 0,
    "AINT", DFLAG_AINT, aint_apply
};

void dialect_register_aint(void)
{
    dialect_register(&aint_config);
}
