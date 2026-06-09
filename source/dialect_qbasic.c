/*
 * dialect_qbasic.c -- QBasic (MS-DOS 5.0+)
 *
 * Microsoft, 1991. Last DOS BASIC. Interpreter-only subset of
 * QuickBASIC 4.5. SUB/FUNCTION, SELECT CASE, DO/LOOP, line
 * numbers optional. Shipped with GORILLAS.BAS and NIBBLES.BAS.
 *
 * TODO: full SUB/FUNCTION local variable scoping
 * TODO: REDIM PRESERVE
 * TODO: SCREEN modes 7-13 (EGA/VGA pixel graphics)
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
