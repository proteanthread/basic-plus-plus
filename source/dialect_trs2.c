/*
 * dialect_trs2.c -- TRS-80 Level II BASIC
 *
 * Microsoft, 1978. 12KB ROM upgrade for the TRS-80 Model I.
 * Full floating point, string arrays, DEF FN, WHILE/WEND.
 * Also used on Model III and Model 4.
 *
 * TODO: SET/RESET/POINT block graphics
 * TODO: CLOAD/CSAVE cassette commands
 * TODO: type suffixes (! single, # double)
 * TODO: VARPTR()
 */

#include "dialect.h"

static const DialectConfig trs2_config = {
    DIALECT_TRS80_L2,
    "TRS-80 Level II BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 1,
    32767, 0, 1, 1, 0, 1, 0, 1, 1, 1,
    "READY", 14, 1, 1, 1,
    "TRS2", DFLAG_TRS2
};

void dialect_register_trs2(void)
{
    dialect_register(&trs2_config);
}
