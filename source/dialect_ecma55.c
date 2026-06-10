/*
 * dialect_ecma55.c -- ECMA-55 Minimal BASIC (ISO 6373:1984)
 *
 * International standard. The portable subset: if it runs under
 * EC55 strict mode, it'll run on any conforming implementation.
 * LET is mandatory, no WHILE, no CLS, no ON ERROR.
 *
 * Missing: OPTION BASE 0|1 (currently always 0-based). To add
 * it, the DIM handler in parser.c would need to store the base
 * in the array header and offset all index calculations. Also
 * missing: strict numeric precision rules (at least 6 significant
 * digits). We already use double, so this is mostly satisfied,
 * but ECMA-55 has specific rounding behavior in PRINT that we
 * don't enforce.
 */

#include "dialect.h"

static const DialectConfig ecma55_config = {
    DIALECT_ECMA55,
    "ECMA-55 Minimal BASIC",
    ':', 1,
    0,                      /* LET mandatory */
    1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY", 14, 0, 0, 0,
    "EC55", DFLAG_EC55
};

void dialect_register_ecma55(void)
{
    dialect_register(&ecma55_config);
}
