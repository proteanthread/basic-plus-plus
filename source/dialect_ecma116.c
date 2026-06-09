/*
 * =====================================================================
 * BASIC++ Dialect Profile — ECMA-116 Full BASIC (E116)
 * =====================================================================
 *
 * STANDARD:  ECMA-116 (1986), ISO 10279:1991
 * AUTHOR:    ECMA Technical Committee TC21
 * YEAR:      1986 (ECMA), 1991 (ISO)
 *
 * HISTORICAL NOTES:
 *   The full international BASIC standard — the gold standard for
 *   portable BASIC. ECMA-116 extends ECMA-55 with structured
 *   control flow, multi-line procedures, exception handling, matrix
 *   operations, and enhanced file I/O.
 *
 *   BASIC++ targets 100% ECMA-116 compliance (excluding OPTION
 *   ARITHMETIC DECIMAL per project directive — we use IEEE 754
 *   binary floating point instead of decimal arithmetic).
 *
 * KEY FEATURES (beyond ECMA-55):
 *   - Structured control: WHILE/WEND, DO/LOOP, SELECT CASE
 *   - Multi-line: SUB/END SUB, FUNCTION/END FUNCTION
 *   - Exception handling: WHEN EXCEPTION IN/USE/END WHEN
 *   - CAUSE EXCEPTION (throw)
 *   - RETRY (restart WHEN block)
 *   - CONTINUE (skip to next iteration)
 *   - DECLARE (forward declarations)
 *   - MAT operations (matrix math)
 *   - Enhanced files: RELATIVE, STREAM organization
 *   - SET/ASK/POINTER/FILESIZE (file control)
 *   - REWRITE (in-place record update)
 *   - ERASE (delete file contents)
 *   - EXIT FOR/DO (break out of loops)
 *   - CONST (named constants)
 *
 * WHAT'S IMPLEMENTED:
 *   [x] WHILE/WEND, DO/LOOP
 *   [x] SELECT CASE / CASE / CASE ELSE / END SELECT
 *   [x] IF/THEN/ELSEIF/ELSE/ENDIF (multi-line IF)
 *   [x] SUB/END SUB, FUNCTION/END FUNCTION
 *   [x] DECLARE (forward declarations)
 *   [x] WHEN/USE/END WHEN exception handling
 *   [x] CAUSE EXCEPTION
 *   [x] RETRY, CONTINUE
 *   [x] MAT operations (MAT PRINT, MAT A=ZER, etc.)
 *   [x] Enhanced files: RELATIVE, STREAM organization
 *   [x] SET POINTER / ASK POINTER / ASK FILESIZE
 *   [x] REWRITE / ERASE
 *   [x] EXIT FOR / EXIT DO
 *   [x] CONST
 *   [x] ON ERROR GOTO / RESUME
 *   [x] SWAP, RANDOMIZE
 *
 * WHAT'S MISSING (for 100% compliance):
 *   [ ] OPTION ARITHMETIC DECIMAL
 *       ECMA-116 specifies decimal floating point as an option.
 *       EXCLUDED BY PROJECT DIRECTIVE. We use IEEE 754 binary.
 *   [ ] OPTION ARITHMETIC NATIVE / FIXED
 *   [ ] Multi-line FUNCTION with local variables
 *       Currently DEF FN is single-expression. Full FUNCTION
 *       blocks with LOCAL declarations are stubbed.
 *       Fix: Implement FUNCTION/END FUNCTION with a local
 *       variable scope stack in runtime.c.
 *   [ ] EXTERNAL (module interface declarations)
 *   [ ] CHAIN with COMMON (shared variable passing between
 *       programs via CHAIN)
 *   [ ] IMAGE statement (formatted I/O templates)
 *   [ ] PRINT USING with IMAGE reference
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit ecma116_config to change feature flags.
 *   2. For multi-line FUNCTION: implement scope stack in
 *      runtime.c, add END FUNCTION handler in parser.c.
 *   3. For IMAGE: add KW_IMAGE to lexer, implement as a
 *      format string stored in the program.
 *   4. Rebuild. Test with: DIALECT "E116"
 *   5. Validate against the ECMA-116 standard document.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig ecma116_config = {
    DIALECT_ECMA116,            /* id */
    "ECMA-116 Full BASIC",      /* name */
    ':',    /* stmt_separator */
    1,      /* has_then_keyword */
    1,      /* has_let_optional */
    1,      /* has_for_next */
    1,      /* has_string_vars */
    0,      /* has_print_hash */
    0,      /* has_array_at */
    1,      /* has_rnd_function */
    1,      /* has_abs_function */
    0,      /* has_size_function */
    99999,  /* max_line_number: 5 digits */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    1,      /* has_while_wend */
    1,      /* has_do_loop */
    1,      /* has_extended_vars */
    1,      /* has_merge_chain */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "READY",    /* ready_prompt */
    14,         /* print_zone_width */
    1,          /* has_on_error */
    0,          /* has_cls: NO (not in standard) */
    0,          /* has_tron_troff: NO (not in standard) */
    "E116",     /* short_name */
    DFLAG_E116  /* dialect_flag */
};

void dialect_register_ecma116(void)
{
    dialect_register(&ecma116_config);
}
