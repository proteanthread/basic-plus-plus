/*
 * dialect_patb.c -- Palo Alto Tiny BASIC
 *
 * Li-Chen Wang, 1976. Published in Dr. Dobb's Journal Vol. 1 No. 1.
 * Ran on Altair 8800, IMSAI 8080, and other S-100 bus systems.
 * 2KB interpreter, integer-only, 26 variables (A-Z).
 *
 * Notable differences from Microsoft BASIC:
 *   - Statement separator is ';' not ':'
 *   - Not-equal operator is '#' not '<>'
 *   - IF has no THEN keyword:  IF A>5 PRINT "BIG"
 *   - Single array via @(I) syntax
 *   - PRINT # sets field width
 *   - No strings, no DATA/READ, no floating point
 *
 * TODO: memory-mapped @() array (original shared with free memory)
 */

#include "dialect.h"

static const DialectConfig patb_config = {
    DIALECT_TINY_BASIC,
    "Palo Alto Tiny BASIC",
    ';',                    /* unique semicolon separator */
    0,                      /* no THEN keyword */
    1, 1, 0,
    1,                      /* PRINT # field width */
    1,                      /* @() array syntax */
    1, 1, 1,
    32767, 1,               /* max line; '#' means not-equal */
    0, 1, 1, 0,
    1, 0, 0, 0,
    "READY", 8, 0, 0, 0,
    "PATB", DFLAG_PATB
};

void dialect_register_patb(void)
{
    dialect_register(&patb_config);
}
