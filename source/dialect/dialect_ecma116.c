/*
 * dialect_ecma116.c -- ECMA-116 Full BASIC (ISO 10279:1991)
 *
 * The gold standard. Structured control flow, SUB/FUNCTION,
 * WHEN/USE exception handling, MAT matrix ops, enhanced files.
 * BASIC++ targets full compliance here (minus OPTION ARITHMETIC
 * DECIMAL -- we stick with IEEE 754 binary float).
 *
 * What's left to implement:
 *   Multi-line FUNCTION with LOCAL scope. Right now DEF FN only
 *   does single-expression functions. To do real FUNCTION, you'd
 *   need a local symbol table pushed onto a scope stack in
 *   runtime.c. Look at how GOSUB saves/restores state and extend
 *   that to include a variable frame.
 *
 *   EXTERNAL declarations (inter-module linking). Would work
 *   through the module system -- see module.h.
 *
 *   IMAGE statement for formatted output. This is printf-style
 *   formatting but with BASIC syntax: IMAGE ###.## etc. Parser
 *   would build a format string and feed it to snprintf.
 *
 *   CHAIN with COMMON (passing variables between programs).
 *   The runtime already has CHAIN; COMMON would need a list of
 *   variable names that survive the CHAIN boundary.
 */

#include "dialect.h"

/*
 * ecma116_apply - Apply ECMA-116 Full BASIC configuration.
 *
 * ECMA-116 is the most complete standards-based dialect:
 *   - Full structured control flow (WHILE, DO, SELECT CASE)
 *   - SUB/FUNCTION with local scope
 *   - WHEN EXCEPTION/USE error handling
 *   - MAT matrix operations
 *   - Enhanced file I/O module
 *   - SET/ASK system configuration
 *   - Has MERGE/CHAIN (has_merge_chain = 1)
 *   - Has ON ERROR (has_on_error = 1)
 *   - No CLS (terminal-independent standard)
 *   - No TRON/TROFF (not in the standard)
 */
static void ecma116_apply(void)
{
 /* ECMA-116 is feature-complete. The WHEN/USE exception
  * handling is implemented in parser_errhand.c and tagged
  * with DFLAG_E116 in the keyword table. CAUSE/RETRY/
  * CONTINUE are the exception control keywords.
  * SET/ASK are the system configuration commands. */
}

static const DialectConfig ecma116_config = {
    DIALECT_ECMA116,
    "ECMA-116 Full BASIC",
    ':', 1, 1, 1, 1, 0, 0, 1, 1, 0,
    99999, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    "READY", 14, 1, 0, 0,
    "E116", DFLAG_E116, ecma116_apply
};

void dialect_register_ecma116(void)
{
    dialect_register(&ecma116_config);
}
