/*
 * =====================================================================
 * BASIC++ Dialect Profile — Palo Alto Tiny BASIC (PATB)
 * =====================================================================
 *
 * MACHINE:   Intel 8080 / Zilog Z80 homebrew computers
 * AUTHOR:    Li-Chen Wang
 * YEAR:      1976
 * ROM SIZE:  ~2KB interpreter
 * RAM:       Runs in 4KB total
 * CPU:       Intel 8080 (also runs on Z80)
 * STORAGE:   Paper tape, cassette, or typed in by hand
 *
 * HISTORICAL NOTES:
 *   The original Tiny BASIC, published in Dr. Dobb's Journal of
 *   Computer Calisthenics & Orthodontia, Vol. 1 No. 1, 1976.
 *   Li-Chen Wang's "Palo Alto" implementation was the most widely
 *   copied version. It ran on Altair 8800, IMSAI 8080, Sol-20,
 *   Cromemco, and countless other S-100 bus systems.
 *
 *   Wang famously copyrighted it as "@COPYLEFT ALL WRONGS RESERVED"
 *   — the original open-source ethos. BASIC++ honors this in its
 *   own copyright string.
 *
 * DIALECT QUIRKS (compared to Microsoft BASIC):
 *   - Statement separator is ';' (not ':')
 *   - Not-equal operator is '#' (not '<>')
 *   - IF has no THEN keyword:  IF A>5 PRINT "BIG"
 *   - Array access uses @():  LET @(I)=42
 *   - Only one array (shared with free memory)
 *   - PRINT # sets field width:  PRINT #8, A
 *   - Integer-only (no floating point)
 *   - 26 variables only (A-Z), no named variables
 *   - No string variables at all
 *   - No DATA/READ/RESTORE
 *   - No DIM (single array via @())
 *   - SIZE function returns free memory
 *   - No CLS, no TRON/TROFF, no ON ERROR
 *
 * WHAT'S IMPLEMENTED:
 *   [x] Semicolon statement separator
 *   [x] Hash not-equal operator
 *   [x] IF without THEN
 *   [x] @() array access
 *   [x] PRINT # field width
 *   [x] Integer arithmetic
 *   [x] 26 single-letter variables
 *   [x] SIZE function
 *   [x] RND, ABS functions
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] Memory-mapped @() array (currently uses DIM-style storage)
 *       The original PATB shared @() with free memory, so @(0) was
 *       the first byte above the program, @(SIZE) was the last.
 *       Fix: Make @() index into the variable memory pool directly.
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit the patb_config struct below to change feature flags.
 *   2. Rebuild BASIC++. No other files need changing.
 *   3. Test with: DIALECT "PATB" then try PATB-specific syntax.
 *   4. For dialect-specific runtime behavior (e.g., @() memory
 *      mapping), add a dialect_patb_apply() function and wire
 *      it into dialect_apply() in dialect.c.
 *
 * HOW TO ADD PATB-SPECIFIC FEATURES:
 *   1. Add the feature flag to DialectConfig in dialect.h.
 *   2. Set it here in patb_config.
 *   3. Query it in parser.c:  dialect_get_config()->has_xxx
 *   4. The parser auto-adapts — no dialect-specific if/else needed.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig patb_config = {
    DIALECT_TINY_BASIC,         /* id */
    "Palo Alto Tiny BASIC",     /* name */
    ';',    /* stmt_separator: semicolon (unique to PATB) */
    0,      /* has_then_keyword: NO — IF A>5 PRINT "BIG" */
    1,      /* has_let_optional: YES — A=5 is valid */
    1,      /* has_for_next: YES */
    0,      /* has_string_vars: NO — no string support at all */
    1,      /* has_print_hash: YES — PRINT #8, A = 8-wide field */
    1,      /* has_array_at: YES — @(I) array syntax */
    1,      /* has_rnd_function: YES — RND(N) returns 1..N */
    1,      /* has_abs_function: YES */
    1,      /* has_size_function: YES — SIZE returns free memory */
    32767,  /* max_line_number: 15-bit signed integer max */
    1,      /* not_eq_is_hash: YES — A#B means A<>B */
    0,      /* has_data_read: NO */
    1,      /* has_while_wend: YES (BASIC++ extension) */
    1,      /* has_do_loop: YES (BASIC++ extension) */
    0,      /* has_extended_vars: NO — A-Z only */
    1,      /* has_merge_chain: YES */
    0,      /* has_float: NO — integer only */
    0,      /* has_dim_arrays: NO — uses @() instead */
    0,      /* has_string_functions: NO */
    /* Phase 8: Dialect personality */
    "READY",    /* ready_prompt */
    8,          /* print_zone_width: 8 columns */
    0,          /* has_on_error: NO */
    0,          /* has_cls: NO */
    0,          /* has_tron_troff: NO */
    "PATB",     /* short_name: 4-char dialect code */
    DFLAG_PATB  /* dialect_flag: bitmask for keyword filtering */
};

void dialect_register_patb(void)
{
    dialect_register(&patb_config);
}
