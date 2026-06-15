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

/*
 * ecma55_apply - Apply ECMA-55 Minimal BASIC configuration.
 *
 * ECMA-55 is a standards-compliant minimal BASIC:
 *   - LET is mandatory (has_let_optional = 0)
 *   - Has strings but only single-letter A$-Z$
 *   - No named/extended variables beyond A-Z, A$-Z$
 *   - No WHILE/WEND, no DO/LOOP, no ON ERROR
 *   - No CLS, no TRON/TROFF
 *   - No MERGE/CHAIN
 *   - Max line number 99999
 *   - Has DATA/READ/RESTORE, DIM, DEF FN
 *   - Floats are available (has_float = 1)
 *   - GOTO into FOR body is a warning in strict mode
 */
static void ecma55_apply(void)
{
 /* ECMA-55 mandates LET (enforced by has_let_optional = 0
  * and the parser's LET-required rejection).
  * GOTO-into-FOR warnings are handled in parser_flow.c.
  * Numeric precision defaults to double (>= 6 digits). */
}

static const DialectConfig ecma55_config = {
    DIALECT_ECMA55,
    "ECMA-55 Minimal BASIC",
    ':', 1,
    0,                      /* LET mandatory */
    1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 0, 0, 1, 0, 1, 1, 1,
    "READY", 14, 0, 0, 0,
    "EC55", DFLAG_EC55, ecma55_apply
};

void dialect_register_ecma55(void)
{
    dialect_register(&ecma55_config);
}
