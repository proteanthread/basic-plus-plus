/*
 * =====================================================================
 * BASIC++ Interpreter - dialect.c
 * =====================================================================
 *
 * Dialect system implementation - Full Dialect Profiles (Phase 8).
 *
 * DESIGN RATIONALE:
 *   All dialect-specific behavior is configured through a static
 *   table of DialectConfig structs. The active dialect is selected
 *   at startup (or runtime) by setting a pointer to the appropriate
 *   config entry. The parser and executor never contain dialect-
 *   specific conditionals - they query this module instead.
 *
 *   Phase 8 adds historically accurate profiles for all 12
 *   supported dialects, with proper ready prompts, PRINT zone
 *   widths, feature flags, and dialect-specific personality.
 *
 * HISTORICAL ACCURACY NOTES:
 *   Each dialect config below was researched against original
 *   documentation and reference implementations:
 *
 *   - PATB: Li-Chen Wang, 1976. Uses ';' separator, # for !=,
 *     no THEN keyword, @() arrays, 26 variables (A-Z), no strings.
 *
 *   - TRS-80 Level I: Based on Tiny BASIC by Steve Leininger.
 *     Very limited: 26 numeric vars, basic string support via A$,
 *     no string arrays, 2-byte integers. SET/RESET for graphics.
 *
 *   - TRS-80 Level II: Microsoft BASIC. Full float, string arrays,
 *     DIM, DEF FN, ON GOTO/GOSUB, DATA/READ, multi-statement lines.
 *
 *   - GW-BASIC: Microsoft, 1983. IBM PC BASIC variant. Full float,
 *     strings, arrays, WHILE/WEND, ON ERROR, SWAP, LINE INPUT.
 *
 *   - ECMA-55: 1978 standard. Minimal BASIC. Requires LET keyword,
 *     no multi-statement lines originally, but we allow ':'.
 *
 *   - ECMA-116: 1986 standard. Full BASIC. Structured control flow,
 *     multi-line functions, exception handling.
 *
 *   - QBasic: Microsoft, 1991. Structured BASIC. SUB/FUNCTION,
 *     SELECT CASE, long variable names, no line numbers required.
 *
 *   - Apple II Integer BASIC: Wozniak, 1977. Integer-only, no
 *     floating point, limited string support, 16-bit integers.
 *
 *   - AppleSoft BASIC: Microsoft for Apple II, 1977. Full float,
 *     standard Microsoft BASIC feature set.
 *
 *   - Atari BASIC: Shepardson Microsystems, 1979. Tokenized storage,
 *     no LET required, CLR clears vars, DIM required for strings.
 *
 *   - Commodore BASIC v2: Microsoft, 1982 (C64). Based on MS BASIC
 *     6502. Full float, limited error handling, PEEK/POKE/SYS.
 *
 *   - CoCo BASIC: Microsoft Extended Color BASIC (Tandy CoCo).
 *     Similar to TRS-80 Level II with color/sound extensions.
 *
 * HOW TO ADD A NEW DIALECT:
 *   1. Add to DialectId enum in dialect.h.
 *   2. Add a DialectConfig entry below with accurate feature flags.
 *   3. Set ready_prompt, print_zone_width, short_name.
 *   4. No parser changes needed - auto-adapts via config queries.
 *
 * HOW TO WRITE EXTERNAL DIALECT MODULES:
 *   External modules can create custom dialects by:
 *   1. Calling dialect_init() with a base dialect ID.
 *   2. Using funcreg_override() to swap function handlers.
 *   3. The module system (future Phase 14) will formalize this.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "dialect.h"

/* =====================================================================
 * Dialect Configuration Table
 * =====================================================================
 * Each entry defines the complete behavior profile for one dialect.
 * Fields are documented in dialect.h (DialectConfig).
 *
 * The table is indexed by DialectId. Order must match the enum.
 *
 * Field order per row:
 *   id, name, stmt_sep,
 *   has_then, has_let_opt, has_for, has_string, has_print_hash,
 *   has_array_at, has_rnd, has_abs, has_size, max_line, not_eq_hash,
 *   has_data, has_while, has_do, has_ext_vars, has_merge,
 *   has_float, has_dim, has_strfuncs,
 *   ready_prompt, zone_width, has_on_error, has_cls, has_tron,
 *   short_name, dialect_flag
 */
static const DialectConfig dialect_configs[DIALECT_COUNT] = {
    /* =========================================================
     * DIALECT_TINY_BASIC - Palo Alto Tiny BASIC (Li-Chen Wang)
     * =========================================================
     * The original and default dialect. Minimal, elegant, pure.
     * Uses ';' separator, # for not-equal, @() arrays, no strings.
     */
    {
        DIALECT_TINY_BASIC,
        "Palo Alto Tiny BASIC",
        ';',    /* stmt_separator: semicolon (unique to PATB) */
        0,      /* has_then_keyword: NO (IF expr relop expr stmt) */
        1,      /* has_let_optional: YES (A=5 is valid) */
        1,      /* has_for_next: YES */
        0,      /* has_string_vars: NO (original PATB) */
        1,      /* has_print_hash: YES (#n format width) */
        1,      /* has_array_at: YES (@(i) array) */
        1,      /* has_rnd_function: YES (RND(x)) */
        1,      /* has_abs_function: YES (ABS(x)) */
        1,      /* has_size_function: YES (SIZE) */
        32767,  /* max_line_number */
        1,      /* not_eq_is_hash: YES (# means <>) */
        0,      /* has_data_read: NO */
        1,      /* has_while_wend: YES (BASIC++ extension) */
        1,      /* has_do_loop: YES (BASIC++ extension) */
        0,      /* has_extended_vars: NO (A-Z only) */
        1,      /* has_merge_chain: YES */
        0,      /* has_float: NO (integer only) */
        0,      /* has_dim_arrays: NO */
        0,      /* has_string_functions: NO */
        /* Phase 8 */
        "READY",  /* ready_prompt */
        8,        /* print_zone_width: 8 columns */
        0,        /* has_on_error: NO */
        0,        /* has_cls: NO (original PATB) */
        0,        /* has_tron_troff: NO */
        "PATB",    /* short_name */
        DFLAG_PATB /* dialect_flag */
    },

    /* =========================================================
     * DIALECT_TRS80_L1 - TRS-80 Level I BASIC
     * =========================================================
     * Steve Leininger's Tiny BASIC variant. Very limited but
     * historically significant. Integer-only, basic string A$.
     * SET/RESET/POINT for block graphics (not emulated here).
     */
    {
        DIALECT_TRS80_L1,
        "TRS-80 Level I BASIC",
        ':',    /* stmt_separator: colon */
        1,      /* has_then: YES */
        1,      /* has_let_optional: YES */
        1,      /* has_for_next: YES */
        1,      /* has_string_vars: YES (A$ only - 1 string var) */
        0,      /* has_print_hash: NO */
        0,      /* has_array_at: NO (uses DIM A(n)) */
        1,      /* has_rnd: YES (RND(n)) */
        1,      /* has_abs: YES */
        1,      /* has_size: YES (MEM on real TRS-80) */
        32767,  /* max_line_number */
        0,      /* not_eq_is_hash: NO (uses <>) */
        1,      /* has_data_read: YES */
        0,      /* has_while_wend: NO */
        0,      /* has_do_loop: NO */
        0,      /* has_extended_vars: NO */
        0,      /* has_merge_chain: NO */
        0,      /* has_float: NO (integer only, 2 bytes) */
        1,      /* has_dim_arrays: YES */
        1,      /* has_string_functions: YES (limited) */
        /* Phase 8 */
        "READY",  /* ready_prompt */
        16,       /* print_zone_width: 16 columns */
        0,        /* has_on_error: NO */
        1,        /* has_cls: YES (CLS command) */
        0,        /* has_tron_troff: NO */
        "TRS1",    /* short_name */
        DFLAG_TRS1 /* dialect_flag */
    },

    /* =========================================================
     * DIALECT_TRS80_L2 - TRS-80 Level II BASIC
     * =========================================================
     * Full Microsoft BASIC. Major upgrade over Level I.
     * Floating point, full string support, multi-dim arrays.
     */
    {
        DIALECT_TRS80_L2,
        "TRS-80 Level II BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 1, 32767, 0,
        1, 1, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "READY",  14, 1, 1, 1, "TRS2",
        DFLAG_TRS2
    },

    /* =========================================================
     * DIALECT_GW_BASIC - GW-BASIC (Microsoft, 1983)
     * =========================================================
     * The IBM PC BASIC workhorse. Full-featured Microsoft BASIC
     * with WHILE/WEND, ON ERROR, sound, graphics (not emulated).
     * GW stands for "Gee Whiz" (Greg Whitten).
     */
    {
        DIALECT_GW_BASIC,
        "GW-BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 65529, 0,
        1, 1, 1, 1, 1,
        1, 1, 1,
        /* Phase 8 */
        "Ok",  14, 1, 1, 1, "GWBS",
        DFLAG_GWBS
    },

    /* =========================================================
     * DIALECT_ECMA55 - ECMA-55 Minimal BASIC (1978 standard)
     * =========================================================
     * International standard. Requires LET, requires THEN,
     * limited but formally specified. No multi-statement lines
     * in original spec, but we allow ':' for convenience.
     */
    {
        DIALECT_ECMA55,
        "ECMA-55 Minimal BASIC",
        ':',  1, 0, 1, 1, 0, 0, 1, 1, 0, 99999, 0,
        1, 0, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "READY",  14, 0, 0, 0, "EC55",
        DFLAG_EC55
    },

    /* =========================================================
     * DIALECT_ECMA116 - ECMA-116 Full BASIC (1986 standard)
     * =========================================================
     * Full international standard. Structured control flow,
     * multi-line functions, exception handling, matrices.
     */
    {
        DIALECT_ECMA116,
        "ECMA-116 Full BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 99999, 0,
        1, 1, 1, 1, 1,
        1, 1, 1,
        /* Phase 8 */
        "READY",  14, 1, 0, 0, "E116",
        DFLAG_E116
    },

    /* =========================================================
     * DIALECT_QBASIC - QBasic (Microsoft, 1991)
     * =========================================================
     * Structured BASIC with SUB/FUNCTION, SELECT CASE, and
     * long variable names. No line numbers required. The
     * "Ok" prompt matches the MS BASIC family.
     */
    {
        DIALECT_QBASIC,
        "QBasic (subset)",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 65529, 0,
        1, 1, 1, 1, 1,
        1, 1, 1,
        /* Phase 8 */
        "Ok",  14, 1, 1, 1, "QBAS",
        DFLAG_QBAS
    },

    /* =========================================================
     * DIALECT_APPLE_INT - Apple II Integer BASIC (Wozniak, 1977)
     * =========================================================
     * Wozniak's hand-assembled BASIC. Integer-only, no floating
     * point, limited string support. Fast but limited.
     * Uses '>' as prompt (no "READY" - just '>').
     */
    {
        DIALECT_APPLE_INT,
        "Apple II Integer BASIC",
        ':',  1, 0, 1, 0, 0, 0, 1, 1, 0, 32767, 0,
        0, 0, 0, 0, 0,
        0, 1, 0,
        /* Phase 8 */
        ">",  16, 0, 0, 0, "AINT",
        DFLAG_AINT
    },

    /* =========================================================
     * DIALECT_APPLESOFT - AppleSoft BASIC (Microsoft, 1977)
     * =========================================================
     * Microsoft BASIC for Apple II. Full float, standard MS
     * feature set. Slow (9-digit float in ROM) but capable.
     */
    {
        DIALECT_APPLESOFT,
        "AppleSoft BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 63999, 0,
        1, 0, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "]",  16, 1, 0, 0, "ASFT",
        DFLAG_ASFT
    },

    /* =========================================================
     * DIALECT_ATARI_MS - Atari/Microsoft BASIC II (1979)
     * =========================================================
     * Shepardson Microsystems for Atari 400/800. Tokenized source
     * storage, no LET required, CLR clears variables. DIM required
     * for strings (DIM A$(20)). TRAP for error handling.
     */
    {
        DIALECT_ATARI_MS,
        "Atari BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 32767, 0,
        1, 0, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "READY",  10, 1, 0, 1, "ATRI",
        DFLAG_ATRI
    },

    /* =========================================================
     * DIALECT_COMMODORE - Commodore BASIC v2 (Microsoft, 1982)
     * =========================================================
     * Microsoft BASIC 6502 for C64/VIC-20. Full float, limited
     * error handling. Famous for PEEK/POKE/SYS (not emulated).
     * Uses "READY." with period as prompt.
     */
    {
        DIALECT_COMMODORE,
        "Commodore BASIC v2",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 63999, 0,
        1, 0, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "READY.",  10, 0, 0, 0, "C64B",
        DFLAG_C64B
    },

    /* =========================================================
     * DIALECT_COCO - Tandy Color Computer BASIC (Microsoft)
     * =========================================================
     * Microsoft Extended Color BASIC for TRS-80 Color Computer.
     * Similar to Level II with color/sound graphics extensions.
     * Uses "OK" as prompt (uppercase).
     */
    {
        DIALECT_COCO,
        "Color Computer BASIC",
        ':',  1, 1, 1, 1, 0, 0, 1, 1, 0, 63999, 0,
        1, 0, 0, 1, 0,
        1, 1, 1,
        /* Phase 8 */
        "OK",  16, 1, 1, 1, "COCO",
        DFLAG_COCO
    }
};

/* =====================================================================
 * Module State
 * =====================================================================
 * active_dialect: Pointer to the currently selected dialect config.
 * Initialized to Tiny BASIC by default.
 */
static const DialectConfig *active_dialect = &dialect_configs[0];

/* =====================================================================
 * Public API
 * =====================================================================
 */

/*
 * dialect_init - Select the active dialect.
 *
 * Validates the dialect ID and sets the active_dialect pointer.
 * Falls back to DIALECT_TINY_BASIC for invalid IDs.
 */
void dialect_init(DialectId id)
{
    if (id >= 0 && id < DIALECT_COUNT) {
        active_dialect = &dialect_configs[id];
    } else {
        active_dialect = &dialect_configs[DIALECT_TINY_BASIC];
    }
}

/*
 * dialect_get_config - Return the active dialect configuration.
 */
const DialectConfig *dialect_get_config(void)
{
    return active_dialect;
}

/*
 * dialect_get_name - Return the display name of the active dialect.
 */
const char *dialect_get_name(void)
{
    return active_dialect->name;
}

/*
 * dialect_get_separator - Return the statement separator character.
 */
char dialect_get_separator(void)
{
    return active_dialect->stmt_separator;
}

/*
 * dialect_get_ready_prompt - Return the ready prompt text.
 *
 * Each dialect had its own characteristic prompt:
 *   PATB: "READY"    GW-BASIC/QBasic: "Ok"
 *   Commodore: "READY."   CoCo: "OK"
 *   AppleSoft: "]"    Apple Integer: ">"
 */
const char *dialect_get_ready_prompt(void)
{
    return active_dialect->ready_prompt;
}

/*
 * dialect_get_zone_width - Return PRINT zone width for commas.
 */
int dialect_get_zone_width(void)
{
    return active_dialect->print_zone_width;
}

/*
 * dialect_get_short_name - Return 4-character dialect code.
 */
const char *dialect_get_short_name(void)
{
    return active_dialect->short_name;
}

/*
 * dialect_list_all - Print all available dialect names.
 *
 * Shows dialect ID, name, short code, and active indicator.
 */
void dialect_list_all(void)
{
    int i;
    printf("Available dialects:\n");
    for (i = 0; i < DIALECT_COUNT; i++) {
        printf("  %2d: %-30s [%s]%s\n",
               i,
               dialect_configs[i].name,
               dialect_configs[i].short_name,
               (&dialect_configs[i] == active_dialect) ? " *" : "");
    }
}

/*
 * dialect_find_by_name - Look up a dialect by name.
 *
 * Case-insensitive substring match against dialect names
 * and short names.
 * Returns the DialectId, or -1 if not found.
 */
int dialect_find_by_name(const char *name)
{
    int i;
    int name_len;

    if (name == NULL) return -1;

    name_len = (int)strlen(name);
    if (name_len == 0) return -1;

    for (i = 0; i < DIALECT_COUNT; i++) {
        const char *dn = dialect_configs[i].name;
        const char *sn = dialect_configs[i].short_name;
        int dn_len = (int)strlen(dn);
        int sn_len = (int)strlen(sn);
        int j;

        /* Check short_name exact match first (case-insensitive) */
        if (name_len == sn_len) {
            int k, match = 1;
            for (k = 0; k < sn_len; k++) {
                char a = name[k];
                char b = sn[k];
                if (a >= 'a' && a <= 'z') a = (char)(a - 32);
                if (b >= 'a' && b <= 'z') b = (char)(b - 32);
                if (a != b) { match = 0; break; }
            }
            if (match) return i;
        }

        /* Check full name substring match */
        for (j = 0; j <= dn_len - name_len; j++) {
            int k;
            int match = 1;
            for (k = 0; k < name_len; k++) {
                char a = name[k];
                char b = dn[j + k];
                if (a >= 'a' && a <= 'z') a = (char)(a - 32);
                if (b >= 'a' && b <= 'z') b = (char)(b - 32);
                if (a != b) { match = 0; break; }
            }
            if (match) return i;
        }
    }

    return -1;
}

/*
 * dialect_apply - Apply dialect-specific overrides.
 *
 * Called after dialect_init() to reconfigure the runtime for the
 * new dialect. Currently a placeholder for Phase 14 (Module System)
 * which will add function registry overrides per dialect.
 *
 * Future: This will call funcreg_override() for dialect-specific
 * function behavior (e.g., Commodore PRINT formatting differences).
 */
void dialect_apply(void)
{
    /*
     * Phase 8: No function overrides yet - all dialects use the
     * same built-in function implementations. The registry
     * override mechanism (funcreg_override) is ready for use
     * when dialect-specific function behavior is needed.
     *
     * Future dialect-specific overrides will be added here:
     *   - Commodore: Different RND behavior (RND(0) = repeat)
     *   - Atari: CLR resets variables (already implemented)
     *   - Apple Integer: No float functions
     */
}

/* =====================================================================
 * Strict Mode
 * =====================================================================
 * When strict_mode is 1, only keywords that belong to the active
 * dialect's bitmask are allowed. In union mode (0, default),
 * all keywords from all dialects are accepted.
 */
static int strict_mode = 0;

/*
 * dialect_set_strict - Enable or disable strict mode.
 */
void dialect_set_strict(int on)
{
    strict_mode = (on != 0) ? 1 : 0;
}

/*
 * dialect_is_strict - Query strict mode.
 */
int dialect_is_strict(void)
{
    return strict_mode;
}

/*
 * dialect_get_flag - Return the active dialect's bitmask.
 */
unsigned int dialect_get_flag(void)
{
    return active_dialect->dialect_flag;
}

/*
 * dialect_keyword_allowed - Check if keyword is permitted.
 *
 * Union mode: always returns 1.
 * Strict mode: checks if the keyword's dialect bitmask
 * includes the active dialect's flag.
 *
 * Uses lexer_get_keyword_flags() to look up the keyword's
 * supported dialects.
 */
int dialect_keyword_allowed(KeywordId kw)
{
    unsigned int kw_flags;

    /* Union mode: everything allowed */
    if (!strict_mode) return 1;

    /* Look up keyword's dialect flags */
    kw_flags = lexer_get_keyword_flags(kw);

    /* DFLAG_ALL keywords are always allowed */
    if (kw_flags == DFLAG_ALL) return 1;

    /* Check if active dialect's bit is set */
    return (kw_flags & active_dialect->dialect_flag) ? 1 : 0;
}
