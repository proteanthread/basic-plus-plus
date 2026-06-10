/*
 * dialect_qbasic.c -- QBasic (Microsoft, 1991)
 *
 * Shipped with MS-DOS 5.0+. Interpreter-only subset of QuickBASIC
 * 4.5. Line numbers optional, SUB/FUNCTION, SELECT CASE, DO/LOOP.
 * Came with GORILLAS.BAS and NIBBLES.BAS.
 *
 * Missing pieces:
 *   Full SUB/FUNCTION scoping -- QBasic isolates local variables
 *   from the main program. Current DEF FN doesn't do that. Need
 *   a proper call frame on the VM stack (runtime.c) with its own
 *   symbol table. Look at how GOSUB works and add a var frame.
 *
 *   REDIM PRESERVE -- resize an array without losing contents.
 *   Would need realloc in the array backing store (runtime.c,
 *   near rt_dim_array).
 *
 *   SCREEN modes 7-13 (EGA/VGA): these map to gfxbuf resolutions.
 *   SCREEN 12 = 640x480x16, SCREEN 13 = 320x200x256. The gfxbuf
 *   already supports arbitrary resolutions; you'd just add the
 *   mode table and palette.
 */

#include "dialect.h"

static const DialectConfig qbasic_config = {
    DIALECT_QBASIC,
    "QBasic (subset)",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    65529, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    "Ok", 14, 1, 1, 1,
    "QBAS", DFLAG_QBAS
};

void dialect_register_qbasic(void)
{
    dialect_register(&qbasic_config);
}
