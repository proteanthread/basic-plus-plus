/*
 * =====================================================================
 * BASIC++ Dialect Profile — QBasic (QBAS)
 * =====================================================================
 *
 * MACHINE:   IBM PC/AT and compatibles (MS-DOS 5.0+)
 * AUTHOR:    Microsoft
 * YEAR:      1991
 * RAM:       Full conventional memory (~640KB)
 * CPU:       Intel 80286 and above
 * DISPLAY:   Text and graphics (SCREEN 0-13)
 * IDE:       Built-in full-screen editor with syntax highlighting
 *
 * HISTORICAL NOTES:
 *   The last Microsoft BASIC for DOS. QBasic shipped free with
 *   MS-DOS 5.0 through 6.22 and Windows 95/98 (on the CD).
 *   Evolved from QuickBASIC 4.5 but WITHOUT the standalone
 *   compiler — QBasic is interpreter-only.
 *
 *   QBasic is a structured BASIC: SUB/FUNCTION procedures,
 *   SELECT CASE, long variable names, and line numbers are
 *   optional. It's the bridge between classic line-numbered
 *   BASIC and modern structured programming.
 *
 *   Many people's last BASIC before moving to C or Visual Basic.
 *   GORILLAS.BAS and NIBBLES.BAS shipped with DOS as examples.
 *
 * DIALECT QUIRKS (compared to GW-BASIC):
 *   - Line numbers optional (but supported)
 *   - SUB name ... END SUB (named procedures)
 *   - FUNCTION name ... END FUNCTION (user functions)
 *   - SELECT CASE ... CASE ... CASE ELSE ... END SELECT
 *   - DO ... LOOP WHILE/UNTIL (plus WHILE/WEND)
 *   - EXIT FOR, EXIT DO, EXIT SUB, EXIT FUNCTION
 *   - DIM SHARED (module-level variables)
 *   - STATIC keyword (persistent local variables)
 *   - REDIM (resize arrays)
 *   - CONST name = value (named constants)
 *   - DECLARE SUB/FUNCTION (forward declarations)
 *   - TYPE ... END TYPE (user-defined types)
 *   - SLEEP n (pause execution)
 *   - SEEK #n, pos (file position)
 *   - LCASE$/UCASE$/LTRIM$/RTRIM$ string functions
 *
 * WHAT'S IMPLEMENTED:
 *   [x] All GW-BASIC features (superset)
 *   [x] SUB/END SUB, FUNCTION/END FUNCTION
 *   [x] SELECT CASE / CASE / CASE ELSE / END SELECT
 *   [x] DO/LOOP WHILE/UNTIL
 *   [x] EXIT FOR/DO/SUB/FUNCTION
 *   [x] DECLARE
 *   [x] TYPE/END TYPE (user-defined types)
 *   [x] CONST
 *   [x] SLEEP
 *   [x] REDIM
 *   [x] SHARED, STATIC keywords
 *   [x] SEEK
 *   [x] LCASE$/UCASE$/LTRIM$/RTRIM$
 *
 * WHAT'S MISSING (for 100% compatibility):
 *   [ ] Full SUB/FUNCTION scoping
 *       QBasic has true local variables in SUB/FUNCTION blocks.
 *       Currently BASIC++ uses a single global variable namespace.
 *       Fix: Implement a variable scope stack in runtime.c that
 *       pushes/pops on SUB entry/exit.
 *   [ ] DIM SHARED vs DIM (module-level vs local)
 *   [ ] STATIC arrays (persistent between SUB calls)
 *   [ ] REDIM PRESERVE (resize array keeping data)
 *   [ ] ON TIMER(n) GOSUB / TIMER ON/OFF/STOP
 *   [ ] DEF SEG / BSAVE / BLOAD for screen memory
 *   [ ] Full SCREEN modes 7-13 (EGA/VGA pixel graphics)
 *       Currently only text-mode canvas rendering.
 *   [ ] GET/PUT for screen region save/restore
 *
 * HOW TO MODIFY THIS FILE:
 *   1. Edit qbasic_config to change feature flags.
 *   2. For variable scoping: implement scope stack in runtime.c,
 *      push on SUB entry, pop on END SUB/EXIT SUB.
 *   3. For SCREEN modes: extend gfxbuf.c with proper resolution
 *      tables (320x200, 640x200, 640x350, 640x480).
 *   4. Rebuild. Test with: DIALECT "QBAS"
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include "dialect.h"

static const DialectConfig qbasic_config = {
    DIALECT_QBASIC,             /* id */
    "QBasic (subset)",          /* name */
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
    65529,  /* max_line_number */
    0,      /* not_eq_is_hash */
    1,      /* has_data_read */
    1,      /* has_while_wend */
    1,      /* has_do_loop */
    1,      /* has_extended_vars */
    1,      /* has_merge_chain */
    1,      /* has_float */
    1,      /* has_dim_arrays */
    1,      /* has_string_functions */
    "Ok",       /* ready_prompt: same as GW-BASIC */
    14,         /* print_zone_width */
    1,          /* has_on_error */
    1,          /* has_cls */
    1,          /* has_tron_troff */
    "QBAS",     /* short_name */
    DFLAG_QBAS  /* dialect_flag */
};

void dialect_register_qbasic(void)
{
    dialect_register(&qbasic_config);
}
