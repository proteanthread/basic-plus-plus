/*
 * dialect_trs2.c -- TRS-80 Level II BASIC (Microsoft, 1978)
 *
 * 12KB ROM upgrade. Full Microsoft BASIC with float, DEF FN,
 * string arrays, WHILE/WEND, ON ERROR. Also Model III/4.
 *
 * The big gap here is type suffixes: Level II distinguished A!
 * (single precision) from A# (double). To support this properly
 * the lexer would need to strip the suffix and tag the variable
 * entry in the symbol table with a precision field. See lexer.c
 * near token_read_identifier().
 *
 * SET/RESET/POINT should share the TRS1 implementation once
 * that gets written. CLOAD/CSAVE can just alias LOAD/SAVE.
 */

#include "dialect.h"

static const DialectConfig trs2_config = {
    DIALECT_TRS80_L2,
    "TRS-80 Level II BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 1,
    32767, 0, 1, 1, 0, 1, 0, 1, 1, 1,
    "READY", 14, 1, 1, 1,
    "TRS2", DFLAG_TRS2, 0
};

void dialect_register_trs2(void)
{
    dialect_register(&trs2_config);
}
