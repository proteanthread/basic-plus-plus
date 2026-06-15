/*
 * dialect_patb.c -- Palo Alto Tiny BASIC (Li-Chen Wang, 1976)
 *
 * The original hobbyist BASIC. Dr. Dobb's Journal, Vol. 1 No. 1.
 * 2KB interpreter for 4KB systems. Integer-only, 26 vars, no strings.
 *
 * Unique syntax: ';' separates statements, '#' means not-equal,
 * IF lacks THEN, arrays use @(). PRINT # sets field width.
 *
 * Modifying this dialect:
 *   Change flags below and rebuild. If you need new keywords, add them
 *   to lexer.h/lexer.c with DFLAG_PATB, then handle in parser.c.
 *   Example: SIZE could be made to return actual pool usage by hooking
 *   into mem_pool_usage() -- see memory.h.
 *
 * Not yet done:
 *   - @() should share memory with the variable pool (original behavior
 *     used the space between the program and the stack). Could be done
 *     by pointing @() into RuntimeState.named_vars directly.
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
    "PATB", DFLAG_PATB, 0
};

void dialect_register_patb(void)
{
    dialect_register(&patb_config);
}
