/*
 * dialect_superbasic.c -- Sinclair QL SuperBASIC (1984)
 *
 * Jan Jones's structured BASIC for the QL. Runs on QDOS with a
 * Motorola 68008 at 7.5 MHz. Notable for being one of the first
 * second-generation BASICs shipped in a consumer micro's ROM,
 * adding named procedures, local variables, and block control
 * structures years before QBasic arrived on the PC.
 *
 * SuperBASIC quirks vs Spectrum BASIC:
 *   - LET is optional (like GW-BASIC, unlike Spectrum)
 *   - REPeat/END REPeat instead of WHILE/WEND
 *   - SELect ON instead of ON...GOTO with computed branches
 *   - Multi-line IF/ELSE/END IF (the Spectrum only had single-line)
 *   - DEFine PROCedure / DEFine FuNction with LOCal scoping
 *   - 7-digit floating point (BYTE magazine called this inadequate
 *     for business: "not the pennies on your balance sheet")
 *   - Colon separator (same as most BASICs, unlike PATB semicolon)
 *   - 32767 max line number
 */

#include "dialect.h"

static const DialectConfig superbasic_config = {
    DIALECT_SUPERBASIC,     /* id */
    "SuperBASIC",           /* name */
    ':',                    /* stmt_separator */
    1,                      /* has_then_keyword */
    1,                      /* has_let_optional */
    1,                      /* has_for_next */
    1,                      /* has_string_vars */
    1,                      /* has_print_hash */
    0,                      /* has_array_at */
    1,                      /* has_rnd_function */
    1,                      /* has_abs_function */
    0,                      /* has_size_function */
    32767,                  /* max_line_number */
    0,                      /* not_eq_is_hash */
    1,                      /* has_data_read */
    0,                      /* has_while_wend -- uses REPeat */
    0,                      /* has_do_loop */
    1,                      /* has_extended_vars */
    0,                      /* has_merge_chain */
    1,                      /* has_float */
    1,                      /* has_dim_arrays */
    1,                      /* has_string_functions */
    "SuperBASIC",           /* ready_prompt */
    16,                     /* print_zone_width */
    1,                      /* has_on_error (WHEN ERRor) */
    1,                      /* has_cls */
    1,                      /* has_tron_troff */
    "SUPB",                 /* short_name */
    DFLAG_SUPA,             /* dialect_flag */
    0                    /* apply_fn */
};

void dialect_register_superbasic(void)
{
    dialect_register(&superbasic_config);
}
