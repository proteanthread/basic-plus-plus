/*
 * dialect_ecma116.c -- ECMA-116 Full BASIC (1986)
 *
 * International standard (ISO 10279:1991). Extends ECMA-55 with
 * structured control flow, SUB/FUNCTION, exception handling
 * (WHEN/USE/END WHEN), MAT operations, enhanced file I/O.
 *
 * BASIC++ targets 100% ECMA-116 compliance (excluding OPTION
 * ARITHMETIC DECIMAL -- we use IEEE 754 binary floating point).
 *
 * TODO: multi-line FUNCTION with LOCAL variables
 * TODO: EXTERNAL module declarations
 * TODO: CHAIN with COMMON
 * TODO: IMAGE statement for formatted I/O
 */

#include "dialect.h"

static const DialectConfig ecma116_config = {
    DIALECT_ECMA116,
    "ECMA-116 Full BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    "READY", 14, 1, 0, 0,
    "E116", DFLAG_E116
};

void dialect_register_ecma116(void)
{
    dialect_register(&ecma116_config);
}
