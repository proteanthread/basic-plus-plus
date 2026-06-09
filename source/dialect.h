/*
 * =====================================================================
 * BASIC++ Interpreter - dialect.h
 * =====================================================================
 *
 * Dialect system interface.
 *
 * PURPOSE:
 *   Centralizes all BASIC dialect-specific behavior. The parser and
 *   executor are dialect-agnostic - they query this module to
 *   determine syntax rules, available features, and compatibility
 *   behavior.
 *
 * WHY THIS EXISTS:
 *   Different BASIC dialects have subtly different syntax rules:
 *   - PATB uses ';' as statement separator; most others use ':'
 *   - PATB's IF has no THEN keyword; most others require it
 *   - Some dialects allow omitting LET; others require it
 *   - Line number ranges vary
 *   - Feature availability varies (FOR/NEXT, strings, etc.)
 *   - Ready prompts differ ("READY", "Ok", "READY.", ">")
 *   - PRINT zone widths differ (8, 14, 16)
 *   - Error message styles vary (short vs verbose)
 *
 *   Rather than scattering if/else checks for each dialect
 *   throughout the parser, all dialect logic lives here. The
 *   parser calls dialect_has_feature() or reads DialectConfig
 *   fields to determine behavior.
 *
 * SUPPORTED DIALECTS (Phase 8):
 *   - Palo Alto Tiny BASIC (default)
 *   - TRS-80 Level I BASIC
 *   - TRS-80 Level II BASIC
 *   - GW-BASIC
 *   - ECMA-55 Minimal BASIC
 *   - ECMA-116 Full BASIC
 *   - QBasic (subset)
 *   - Apple II Integer BASIC
 *   - AppleSoft BASIC
 *   - Atari/Microsoft BASIC II
 *   - Commodore BASIC v2
 *   - Tandy Color Computer BASIC
 *
 * HOW TO EXTEND:
 *   1. Add the new dialect to DialectId enum.
 *   2. Add a DialectConfig entry in dialect.c's dialect_configs[].
 *   3. Set all feature flags appropriately.
 *   4. No parser changes needed - the parser auto-adapts.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_DIALECT_H
#define BASICPP_DIALECT_H

#include "lexer.h"   /* KeywordId */

/* =====================================================================
 * Dialect Bitmask Flags
 * =====================================================================
 * Each keyword is tagged with a bitmask indicating which dialects
 * support it. In union mode (default) all keywords are accepted.
 * In strict mode (OPTION STRICT) only keywords whose bitmask
 * includes the active dialect are allowed.
 *
 * 16-bit unsigned: one bit per dialect (12 used, 4 reserved).
 */
#define DFLAG_PATB   (1u << 0)   /* Palo Alto Tiny BASIC */
#define DFLAG_TRS1   (1u << 1)   /* TRS-80 Level I */
#define DFLAG_TRS2   (1u << 2)   /* TRS-80 Level II */
#define DFLAG_GWBS   (1u << 3)   /* GW-BASIC */
#define DFLAG_EC55   (1u << 4)   /* ECMA-55 */
#define DFLAG_E116   (1u << 5)   /* ECMA-116 */
#define DFLAG_QBAS   (1u << 6)   /* QBasic */
#define DFLAG_AINT   (1u << 7)   /* Apple II Integer BASIC */
#define DFLAG_ASFT   (1u << 8)   /* AppleSoft BASIC */
#define DFLAG_ATRI   (1u << 9)   /* Atari/Microsoft BASIC II */
#define DFLAG_C64B   (1u << 10)  /* Commodore BASIC v2 */
#define DFLAG_COCO   (1u << 11)  /* Tandy CoCo BASIC */
#define DFLAG_ALL    (0xFFFFu)   /* all dialects / BASIC++ native */

/* Convenience: Microsoft BASIC family (shared core) */
#define DFLAG_MSBASIC  (DFLAG_TRS2 | DFLAG_GWBS | DFLAG_QBAS | \
                        DFLAG_ASFT | DFLAG_C64B | DFLAG_COCO)

/* Convenience: All Microsoft + both TRS-80 levels */
#define DFLAG_MSALL    (DFLAG_TRS1 | DFLAG_MSBASIC)

/* Convenience: Structured BASIC (WHILE/WEND, SELECT, etc.) */
#define DFLAG_STRUCT   (DFLAG_GWBS | DFLAG_QBAS | DFLAG_E116)

/* Convenience: GW-BASIC + QBasic (most compatible pair) */
#define DFLAG_GWQB     (DFLAG_GWBS | DFLAG_QBAS)


/* =====================================================================
 * Dialect Identifiers
 * =====================================================================
 * Each supported BASIC dialect has a unique identifier.
 * DIALECT_TINY_BASIC is the default and the only fully
 * implemented dialect in Phase 1.
 */
typedef enum DialectId {
    DIALECT_TINY_BASIC = 0,   /* Palo Alto Tiny BASIC (default) */
    DIALECT_TRS80_L1,         /* TRS-80 Level I BASIC */
    DIALECT_TRS80_L2,         /* TRS-80 Level II BASIC */
    DIALECT_GW_BASIC,         /* GW-BASIC */
    DIALECT_ECMA55,           /* ECMA-55 Minimal BASIC */
    DIALECT_ECMA116,          /* ECMA-116 Full BASIC */
    DIALECT_QBASIC,           /* QBasic (subset) */
    DIALECT_APPLE_INT,        /* Apple II Integer BASIC */
    DIALECT_APPLESOFT,        /* AppleSoft BASIC */
    DIALECT_ATARI_MS,         /* Atari/Microsoft BASIC II */
    DIALECT_COMMODORE,        /* Commodore BASIC */
    DIALECT_COCO,             /* Tandy Color Computer BASIC */
    DIALECT_COUNT             /* sentinel - must be last */
} DialectId;

/* =====================================================================
 * Dialect Configuration
 * =====================================================================
 * Contains all dialect-specific flags and settings. The parser
 * and executor read these flags to adjust their behavior.
 *
 * Fields:
 *   id                  - dialect identifier
 *   name                - human-readable name (for display)
 *   stmt_separator      - character separating statements on one
 *                         line (';' for PATB, ':' for most others)
 *   has_then_keyword    - 1 if IF requires THEN, 0 if not (PATB: 0)
 *   has_let_optional    - 1 if LET keyword can be omitted (A=5)
 *   has_for_next        - 1 if FOR/NEXT loops are supported
 *   has_string_vars     - 1 if string variables (A$) are supported
 *   has_print_hash      - 1 if PRINT # format specifier is supported
 *   has_array_at        - 1 if @() array syntax is supported
 *   has_rnd_function    - 1 if RND() function is available
 *   has_abs_function    - 1 if ABS() function is available
 *   has_size_function   - 1 if SIZE function is available
 *   max_line_number     - maximum valid line number
 *   not_eq_is_hash      - 1 if # is the not-equal operator
 */
typedef struct DialectConfig {
    DialectId   id;
    const char *name;
    char        stmt_separator;
    int         has_then_keyword;
    int         has_let_optional;
    int         has_for_next;
    int         has_string_vars;
    int         has_print_hash;
    int         has_array_at;
    int         has_rnd_function;
    int         has_abs_function;
    int         has_size_function;
    int         max_line_number;
    int         not_eq_is_hash;
    /* Phase 3 additions */
    int         has_data_read;     /* DATA/READ/RESTORE support */
    int         has_while_wend;    /* WHILE/WEND loops */
    int         has_do_loop;       /* DO/LOOP loops */
    int         has_extended_vars;  /* named variables beyond A-Z */
    int         has_merge_chain;   /* MERGE/CHAIN commands */
    /* Phase 4 additions */
    int         has_float;          /* floating-point arithmetic */
    int         has_dim_arrays;     /* DIM arrays */
    int         has_string_functions; /* string functions */
    /* Phase 8 additions - dialect personality */
    const char *ready_prompt;       /* ready prompt text */
    int         print_zone_width;   /* PRINT zone width (tab stops) */
    int         has_on_error;       /* ON ERROR GOTO support */
    int         has_cls;            /* CLS command */
    int         has_tron_troff;     /* TRON/TROFF support */
    const char *short_name;         /* 4-char dialect code for DIALECT$ */
    unsigned int dialect_flag;      /* bitmask flag for this dialect */
} DialectConfig;

/* =====================================================================
 * Dialect Functions
 * =====================================================================
 */

/*
 * dialect_init - Set the active dialect.
 *
 * Configures the interpreter to use the specified dialect's rules.
 * Must be called once at startup (default: DIALECT_TINY_BASIC).
 * Can be called again to switch dialects at runtime.
 */
void dialect_init(DialectId id);

/*
 * dialect_get_config - Return the active dialect configuration.
 *
 * Returns a pointer to the current DialectConfig. The returned
 * pointer is valid until dialect_init() is called again.
 */
const DialectConfig *dialect_get_config(void);

/*
 * dialect_get_name - Return the display name of the active dialect.
 */
const char *dialect_get_name(void);

/*
 * dialect_get_separator - Return the statement separator character.
 *
 * Returns ';' for PATB, ':' for most other dialects.
 */
char dialect_get_separator(void);

/*
 * dialect_list_all - Print all available dialect names and IDs.
 *
 * Used by the DIALECT command without arguments to show
 * available options.
 */
void dialect_list_all(void);

/*
 * dialect_find_by_name - Look up a dialect by name (case-insensitive).
 *
 * Returns the DialectId, or -1 if not found.
 */
int dialect_find_by_name(const char *name);

/*
 * dialect_get_ready_prompt - Return the dialect-specific ready prompt.
 *
 * Different BASICs used different prompts:
 *   PATB: "READY"  GW-BASIC: "Ok"  Commodore: "READY."
 */
const char *dialect_get_ready_prompt(void);

/*
 * dialect_get_zone_width - Return PRINT zone width.
 *
 * Classic BASICs used different tab column widths for comma-
 * separated PRINT items. Typical values: 8, 14, 16.
 */
int dialect_get_zone_width(void);

/*
 * dialect_get_short_name - Return 4-char dialect code.
 *
 * Used by DIALECT$ introspection. E.g., "PATB", "TRS1", "GWBS".
 */
const char *dialect_get_short_name(void);

/*
 * dialect_apply - Apply dialect-specific overrides.
 *
 * Called after dialect_init() to apply function registry overrides
 * and other dialect-specific runtime configuration. This is where
 * the Phase 7 function registry integration happens.
 *
 * Phase 8: Reconfigures function availability based on dialect
 * feature flags (e.g., disabling string functions for PATB).
 */
void dialect_apply(void);

/*
 * dialect_set_strict - Enable/disable strict dialect mode.
 *
 * When strict mode is ON, keywords that don't belong to the active
 * dialect's bitmask are rejected (raise WHAT?).
 * When OFF (default), all keywords from all dialects are accepted.
 *
 * Activated via:  OPTION STRICT     (enable)
 *                 OPTION STRICT OFF (disable)
 */
void dialect_set_strict(int on);

/*
 * dialect_is_strict - Query whether strict mode is active.
 */
int dialect_is_strict(void);

/*
 * dialect_keyword_allowed - Check if a keyword is allowed.
 *
 * In union mode: always returns 1.
 * In strict mode: returns 1 only if the keyword's dialect bitmask
 * includes the active dialect's flag.
 *
 * The keyword's dialect flags are looked up from the lexer's
 * keyword table via lexer_get_keyword_flags().
 */
int dialect_keyword_allowed(KeywordId kw);

/*
 * dialect_get_flag - Return the active dialect's bitmask flag.
 */
unsigned int dialect_get_flag(void);

/* =====================================================================
 * Dialect Registration (Phase 21 — Contributor Architecture)
 * =====================================================================
 * Each dialect lives in its own source file (dialect_gwbs.c, etc.)
 * and registers itself via dialect_register() at boot time.
 *
 * This allows contributors to maintain individual dialect files
 * independently and enables conditional compilation (#ifdef) to
 * include or exclude specific dialects for embedded builds.
 */

/*
 * dialect_register - Register a dialect configuration.
 *
 * Called by each dialect_*.c file during dialect_register_all().
 * The config is copied into the internal table at slot config->id.
 * Returns 0 on success, -1 if id is out of range.
 */
int dialect_register(const DialectConfig *config);

/*
 * dialect_register_all - Register all compiled-in dialects.
 *
 * Called once from main.c during boot, before dialect_init().
 * Calls each dialect_register_XXXX() function.
 */
void dialect_register_all(void);

/* Per-dialect registration functions (one per dialect_*.c file) */
void dialect_register_patb(void);
void dialect_register_trs1(void);
void dialect_register_trs2(void);
void dialect_register_gwbs(void);
void dialect_register_ecma55(void);
void dialect_register_ecma116(void);
void dialect_register_qbasic(void);
void dialect_register_aint(void);
void dialect_register_asft(void);
void dialect_register_atari(void);
void dialect_register_c64(void);
void dialect_register_coco(void);

#endif /* BASICPP_DIALECT_H */
