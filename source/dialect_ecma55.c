/*
 * dialect_ecma55.c -- ECMA-55 Minimal BASIC (1978)
 *
 * International standard (ISO 6373:1984). Defines the portable
 * subset that all conforming implementations must support.
 * LET is mandatory. No WHILE/WEND, no CLS, no ON ERROR.
 * Max line 99999 (5 digits). 14-column print zones.
 *
 * TODO: OPTION BASE 0|1
 * TODO: strict numeric precision (6 significant digits minimum)
 */

#include "dialect.h"

static const DialectConfig ecma55_config = {
    DIALECT_ECMA55,
    "ECMA-55 Minimal BASIC",
    ':', 1,
    0,                      /* LET is mandatory */
    1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY", 14, 0, 0, 0,
    "EC55", DFLAG_EC55
};

void dialect_register_ecma55(void)
{
    dialect_register(&ecma55_config);
}
