// ---
// BASIC++ Interpreter - dialect.c
// ---
//
// Dialect system implementation -- Registry Architecture.
//
// PURPOSE:
//   Manages the dialect registry and keyword filtering system.
//   All dialect-specific behavior is configured through DialectConfig
//   structs that are registered at boot time. This module provides:
//   - Dialect registration and lookup
//   - Active dialect selection and configuration
//   - Keyword filtering (Union, Strict, Mixed modes)
//   - Dialect-specific device alias loading
//
// HOW IT WORKS:
//   Each BASIC dialect (GW-BASIC, C64, QBasic, etc.) is defined in
//   its own source file (dialect_gwbs.c, dialect_c64.c, etc.) with
//   a static DialectConfig struct and a registration function.
//
//   Boot sequence:
//     1. dialect_register_all() calls each dialect's register function.
//     2. Each function calls dialect_register() to copy its config
//        into the registry table at the slot matching its DialectId.
//     3. dialect_init(id) selects the active dialect.
//     4. dialect_apply() calls the dialect's apply_fn callback for
//        runtime overrides and loads device aliases.
//
//   At runtime, the parser and executor query this module to determine:
//     - Which keywords are available (via keyword flags + filtering mode)
//     - Syntax rules (statement separator, LET requirement, etc.)
//     - Feature availability (string support, arrays, float, etc.)
//     - Display settings (ready prompt, print zone width, etc.)
//
// ===================================================================
// HOW TO ADD A NEW DIALECT -- Step-by-Step Guide
// ===================================================================
//
//   Step 1: Choose a DialectId.
//     Edit dialect.h and add your dialect to the DialectId enum
//     BEFORE the DIALECT_COUNT entry:
//       DIALECT_YOUR_BASIC,  // Your BASIC v1.0 (1985)
//
//   Step 2: Choose a DFLAG bitmask.
//     Each dialect has a unique bit in the DFLAG_* bitmask system.
//     Add yours to dialect.h:
//       #define DFLAG_YRBAS  (1u << DIALECT_YOUR_BASIC)
//
//   Step 3: Create the dialect file.
//     Create source/dialect/dialect_yourbasic.c containing:
//       - Historical preamble (machine, date, designer)
//       - A static DialectConfig struct with all fields filled in
//       - An apply_fn callback (can be empty if no runtime overrides)
//       - A registration function: dialect_register_yourbasic()
//     Use dialect_sbasic.c as a template -- it's the simplest example.
//
//   Step 4: Fill in the DialectConfig.
//     Set each field to match the real machine's behavior:
//       .id                 = DIALECT_YOUR_BASIC
//       .name               = "Your BASIC"
//       .stmt_separator     = ':'       // or '\\' for some dialects
//       .has_then_keyword   = 1         // 1=supports THEN, 0=no
//       .has_let_optional   = 1         // 1=LET optional, 0=required
//       .has_for_next       = 1         // 1=has FOR/NEXT loops
//       .has_string_vars    = 1         // 1=has A$, 0=integers only
//       .has_print_hash     = 0         // 1=PRINT#n for file output
//       .has_array_at       = 0         // 1=uses @() for arrays
//       .has_rnd_function   = 1         // 1=has RND()
//       .has_abs_function   = 1         // 1=has ABS()
//       .has_size_function  = 0         // 1=has SIZE (free memory)
//       .max_line_number    = 32767     // or 63999, 99999, etc.
//       .not_eq_is_hash     = 0         // 1=uses # for !=
//       .has_data_read      = 1         // 1=has DATA/READ/RESTORE
//       .has_while_wend     = 0         // 1=has WHILE/WEND
//       .has_do_loop        = 0         // 1=has DO/LOOP
//       .has_extended_vars  = 1         // 1=multi-char var names
//       .has_merge_chain    = 0         // 1=has MERGE/CHAIN
//       .has_float          = 1         // 1=floating point support
//       .has_dim_arrays     = 1         // 1=DIM for multi-dim arrays
//       .has_string_functions = 1       // 1=LEFT$/RIGHT$/MID$
//       .ready_prompt       = "Ok"      // prompt after each command
//       .print_zone_width   = 14        // PRINT comma zone width
//       .has_on_error       = 0         // 1=ON ERROR GOTO
//       .has_cls            = 0         // 1=CLS/HOME
//       .has_tron_troff     = 1         // 1=TRON/TROFF
//       .short_name         = "YRBS"    // 4-char abbreviation
//       .dialect_flag       = DFLAG_YRBAS
//       .apply_fn           = yourbasic_apply
//
//   Step 5: Register in dialect_register_all().
//     Add the call to dialect_register_all() below:
//       dialect_register_yourbasic();
//     Add the function declaration in dialect.h:
//       void dialect_register_yourbasic(void);
//
//   Step 6: Add to the build system.
//     In the Makefile, add dialect/dialect_yourbasic.c to
//     DIALECT_SOURCES (and to the MSVC/Watcom source lists).
//
//   Step 7: Tag keywords with your DFLAG.
//     In lexer.c, find any keywords specific to your dialect and
//     add DFLAG_YRBAS to their flags field. For example:
//       {"PEEK", KW_PEEK, DFLAG_C64B | DFLAG_YRBAS, ...}
//
//   That's it! The parser automatically adapts via config queries.
//   No parser code changes are needed.
//
// ===================================================================
// HOW TO FINE-TUNE DIALECT ACCURACY
// ===================================================================
//
//   Each DialectConfig field controls a specific behavior. To make
//   a dialect more accurately match the real machine:
//
//   Feature Fields (has_* flags):
//     Set to 1 to enable, 0 to disable. These control which BASIC
//     statements and functions are available in strict mode.
//     In union mode, all features are available regardless.
//
//   Behavioral Fields:
//     .stmt_separator   -- Character between statements on one line.
//                         Most use ':', Atari uses '\\', TRS-80 L1
//                         uses '\\' (backslash).
//     .has_let_optional -- If 0, LET is required for assignment
//                         (Dartmouth, ECMA-55). If 1, you can write
//                         A = 5 without LET (most dialects).
//     .max_line_number  -- Maximum allowed line number. PATB uses 32767,
//                         GW-BASIC uses 65529, SUPER BASIC uses 99999.
//     .not_eq_is_hash   -- If 1, '#' is the not-equal operator instead
//                         of '<>' (Tiny BASIC convention).
//
//   Display Fields:
//     .ready_prompt     -- The prompt text shown after each command.
//                         GW-BASIC: "Ok", C64: "READY.", CoCo: "OK",
//                         AppleSoft: "]", Integer BASIC: ">".
//     .print_zone_width -- Column width for PRINT comma zones.
//                         Most: 14, GW-BASIC: 14, C64: 10,
//                         SUPER BASIC: 15.
//
//   Runtime Overrides (apply_fn):
//     The apply_fn callback runs when a dialect is selected.
//     Use it for behavior that can't be expressed as a config flag:
//       - Setting default OPTION flags
//       - Configuring RND() seed behavior
//       - Loading dialect-specific function overrides
//       - Adjusting numeric precision modes
//
//   Keyword Flags (DFLAG_*):
//     Each keyword in the lexer table has a bitmask of dialects
//     that support it. In strict mode, only keywords matching the
//     active dialect's flag are allowed. To add a keyword to your
//     dialect, OR your DFLAG into the keyword's flags field.
//
//     Example: To make WHILE available in your dialect:
//       {"WHILE", KW_WHILE, DFLAG_GWBS | DFLAG_QBAS | DFLAG_YRBAS, ...}
//
// ===================================================================
// HOW TO PERSONALIZE YOUR DIALECT / UNION MODE
// ===================================================================
//
//   Union Mode (default):
//     All keywords from all dialects are available. You get GW-BASIC's
//     WHILE/WEND, QBasic's SELECT CASE, SUPER BASIC's COMPLEX, and
//     Atari's TRAP -- all in one interpreter. This is the "BASIC++"
//     experience.
//
//     To customize Union mode, use BASIC statements at runtime:
//       OPTION STRICT OFF           ' ensure union mode
//       KEYWORD DISABLE "WHILE"     ' remove specific keywords
//       KEYWORD ENABLE "UNLESS"     ' add specific keywords
//       OPTION PRINT ZONE 14        ' set comma zone width
//       OPTION LET REQUIRED         ' require LET for assignment
//
//   Strict Mode:
//     Only keywords belonging to the active dialect are accepted.
//     Use this for historical accuracy or compatibility testing:
//       OPTION DIALECT "C64"        ' select Commodore 64 BASIC
//       OPTION STRICT ON            ' enforce C64-only keywords
//
//   Mixed Mode:
//     Combine keywords from specific dialects only:
//       OPTION DIALECT MIXED "GWBS,QBAS,C64B"
//     This allows GW-BASIC + QBasic + C64 keywords but rejects
//     SUPER BASIC's COMPLEX or Atari's TRAP.
//
// TROUBLESHOOTING:
//   - "SORRY? ... is not available in this dialect":
//     You're in strict mode and tried to use a keyword from a
//     different dialect. Either switch to union mode (OPTION STRICT
//     OFF) or add the keyword to your dialect's DFLAG bitmask.
//
//   - Dialect not found:
//     The dialect name/code is not recognized. Use DIALECTS to see
//     the list of available dialects and their short codes.
//
//   - Fallback to default dialect:
//     If dialect_init() receives an invalid or unregistered dialect
//     ID, it falls back to BASICPP_DEFAULT_DIALECT (usually GW-BASIC).
//     This is set in config.h.
//
// PERFORMANCE:
//   - dialect_register(): O(1) -- memcpy into indexed slot.
//   - dialect_get_config(): O(1) -- pointer dereference.
//   - dialect_find_by_name(): O(D x N) where D = dialect count,
//     N = name length. At most 16 dialects, so negligible.
//   - dialect_keyword_allowed(): O(1) -- bitmask AND.
//
// MINIMALIZATION:
//   For a minimal build (FreeDOS, embedded):
//   - Wrap dialect registration calls in #ifndef BPP_FREEDOS.
//   - Only register the dialects you need (e.g., GWBS + ECMA116).
//   - Each dialect file is ~100-200 lines, ~3-5 KB code.
//   - This file (dialect.c) is CORE tier -- always required.
//   - On FreeDOS, only 2 dialects are registered (saves ~20 KB).
//
// DEPENDENCIES:
//   - dialect.h     (DialectConfig, DialectId, DialectMode, DFLAG_*)
//   - config.h      (BASICPP_DEFAULT_DIALECT)
//   - security.h    (SEC_RESTRICTED, SEC_STANDARD)
//   - device_alias.h (device_alias_load_dialect)
//   - lexer.h       (lexer_get_keyword_flags, lexer_keyword_name)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (stdio.h, string.h).
//   - No platform-specific code.
//   - Compiles cleanly on MSVC /std:c17 and gcc -std=c17.
//
// ---

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "dialect.h"
#include "security.h"
#include "device_alias.h"

// -----------------------------------------------------------------
// Dialect Registry Table
// -----------------------------------------------------------------
// Mutable table populated by dialect_register() calls at boot.
// Slots are indexed by DialectId. Empty slots have name == NULL.
//
// The old monolithic static const dialect_configs[] array was
// replaced with this registration pattern so that each dialect
// file owns its own DialectConfig and can be conditionally compiled.
// -----------------------------------------------------------------

static DialectConfig dialect_table[DIALECT_COUNT];
static int dialect_table_count = 0;

// Active dialect pointer -- set by dialect_init().
// All config queries read from this pointer.
static const DialectConfig *active_dialect = NULL;

// -----------------------------------------------------------------
// Registration
// -----------------------------------------------------------------

// dialect_register - Copy a DialectConfig into the registry.
//
// Called by each dialect's register function (e.g.,
// dialect_register_gwbs in dialect_gwbs.c).
//
// Parameters:
//   config - pointer to the DialectConfig to register (copied by value)
//
// Returns:
//   0 on success
//  -1 if config is NULL or has an invalid id
//
int dialect_register(const DialectConfig *config)
{
    if (config == NULL) return -1;
    if (config->id < 0 || config->id >= DIALECT_COUNT) return -1;

    memcpy(&dialect_table[config->id], config, sizeof(DialectConfig));
    dialect_table_count++;
    return 0;
}

// dialect_register_all - Register all compiled-in dialects.
//
// Each registration function is defined in its own dialect_*.c file.
// To add a new dialect, add its register call here and create the file.
//
// On FreeDOS (BPP_FREEDOS defined), only GW-BASIC and ECMA-116
// are registered to fit within 512K conventional memory. To add
// a dialect back for FreeDOS:
//   1. Remove its #ifndef guard here.
//   2. Add its .c file to the Makefile watcom target.
//   3. Verify the binary still fits in conventional memory.
//
// On Windows/Linux, all 16 dialects are always available.
//
void dialect_register_all(void)
{
    // Clear the entire table before registration
    memset(dialect_table, 0, sizeof(dialect_table));
    dialect_table_count = 0;

    // Register each dialect.
    // Order doesn't matter -- each goes into its indexed slot.
#ifndef BPP_FREEDOS
    dialect_register_patb();       // Palo Alto Tiny BASIC (1976)
    dialect_register_trs1();       // TRS-80 Level I BASIC (1977)
    dialect_register_trs2();       // TRS-80 Level II BASIC (1978)
#endif
    dialect_register_gwbs();       // GW-BASIC (1983)
#ifndef BPP_FREEDOS
    dialect_register_ecma55();     // ECMA-55 Minimal BASIC (1978)
#endif
    dialect_register_ecma116();    // ECMA-116 Full BASIC (1986)
#ifndef BPP_FREEDOS
    dialect_register_qbasic();     // Microsoft QBasic (1991)
    dialect_register_aint();       // Apple Integer BASIC (1977)
    dialect_register_asft();       // AppleSoft BASIC (1977)
    dialect_register_atari();      // Atari BASIC (1979)
    dialect_register_c64();        // Commodore 64 BASIC V2 (1982)
    dialect_register_coco();       // TRS-80 Color BASIC (1980)
    dialect_register_mbasic();     // Microsoft MBASIC CP/M (1977)
    dialect_register_sinclair();   // Sinclair ZX Spectrum (1982)
    dialect_register_superbasic(); // Sinclair QL SuperBASIC (1984)
    dialect_register_sbasic();     // Tymshare SUPER BASIC (1968)
#endif
}

// -----------------------------------------------------------------
// Public API -- Dialect Selection
// -----------------------------------------------------------------

// dialect_init - Select the active dialect by ID.
//
// Validates the dialect ID and sets the active_dialect pointer.
// If the ID is invalid or the dialect slot is not registered,
// falls back to BASICPP_DEFAULT_DIALECT (set in config.h).
//
// Parameters:
//   id - DialectId to activate
//
// How to change the default dialect:
//   Edit BASICPP_DEFAULT_DIALECT in config.h. Must be a dialect
//   that is always registered (e.g., DIALECT_GW_BASIC).
//
void dialect_init(DialectId id)
{
    if (id >= 0 && id < DIALECT_COUNT &&
        dialect_table[id].name != NULL) {
        active_dialect = &dialect_table[id];
    } else {
        // Fallback to the compile-time default dialect.
        // On FreeDOS, PATB is not compiled in, so the fallback
        // must be a dialect that IS registered (GWBS or E116).
        active_dialect = &dialect_table[BASICPP_DEFAULT_DIALECT];
    }
}

// dialect_get_config - Return the active dialect's full configuration.
//
// Returns a const pointer to the DialectConfig struct for the
// currently active dialect. Never returns NULL after dialect_init().
//
const DialectConfig *dialect_get_config(void)
{
    return active_dialect;
}

// dialect_get_name - Return the display name of the active dialect.
//
// Examples: "GW-BASIC", "Commodore 64 BASIC V2", "QBasic"
//
const char *dialect_get_name(void)
{
    return active_dialect->name;
}

// dialect_get_separator - Return the statement separator character.
//
// Most dialects use ':' (colon). Some use '\\' (backslash).
// The parser uses this to split multi-statement lines.
//
char dialect_get_separator(void)
{
    return active_dialect->stmt_separator;
}

// dialect_get_ready_prompt - Return the ready prompt text.
//
// Each dialect had its own characteristic prompt on the real machine:
//   PATB:       "READY"
//   GW-BASIC:   "Ok"
//   QBasic:     "Ok"
//   C64:        "READY."
//   CoCo:       "OK"
//   AppleSoft:  "]"
//   Int. BASIC: ">"
//   ECMA-55:    "READY"
//   ECMA-116:   "READY"
//
const char *dialect_get_ready_prompt(void)
{
    return active_dialect->ready_prompt;
}

// dialect_get_zone_width - Return PRINT zone width for commas.
//
// Controls how far the cursor advances on a comma in PRINT:
//   PRINT "A", "B"   -> A at column 0, B at column [zone_width]
//
// Typical values: 14 (GW-BASIC), 10 (C64), 15 (SUPER BASIC).
//
int dialect_get_zone_width(void)
{
    return active_dialect->print_zone_width;
}

// dialect_get_short_name - Return the 4-character dialect code.
//
// Examples: "GWBS", "C64B", "QBAS", "PATB", "SBAS"
// Used for compact displays and the DIALECT$ function.
//
const char *dialect_get_short_name(void)
{
    return active_dialect->short_name;
}

// dialect_list_all - Print all registered dialect names.
//
// Called by the DIALECTS command. Shows each dialect's ID,
// full name, short code, and marks the active one with '*'.
// Also shows "BASIC++ (all features)" as a virtual dialect
// when union mode (non-strict) is active.
//
void dialect_list_all(void)
{
    int i;
    printf("Available dialects:\n");
    for (i = 0; i < DIALECT_COUNT; i++) {
        if (dialect_table[i].name == NULL) continue;
        printf(" %2d: %-30s [%s]%s\n",
            i,
            dialect_table[i].name,
            dialect_table[i].short_name,
            (&dialect_table[i] == active_dialect) ? " *" : "");
    }
    // Show B++ as a virtual dialect (all features enabled)
    printf("  -: %-30s [%s]%s\n",
        "BASIC++ (all features)",
        "B++",
        (dialect_is_strict() == 0) ? " *" : "");
}

// dialect_find_by_name - Look up a dialect by name or code.
//
// Performs case-insensitive matching against:
//   1. Short codes ("GWBS", "C64B", "QBAS", etc.)
//   2. Full names ("GW-BASIC", "Commodore 64", etc.)
//   3. Special names ("BPP", "B++", "BASIC++") -> union mode
//
// Parameters:
//   name - dialect name or code to search for
//
// Returns:
//   DialectId if found (0 to DIALECT_COUNT-1)
//  -1 if not found
//
// For "BPP", "B++", or "BASIC++": disables strict mode and
// returns BASICPP_DEFAULT_DIALECT. This selects the "BASIC++"
// meta-dialect where all features from all dialects are available.
//
int dialect_find_by_name(const char *name)
{
    int i;
    int name_len;

    if (name == NULL) return -1;

    name_len = (int)strlen(name);
    if (name_len == 0) return -1;

    // Check for "BPP", "B++", or "BASIC++" -- these select
    // the BASIC++ meta-dialect (all features, union mode).
    {
        int is_bpp = 0;
        // Check "BPP" (case-insensitive)
        if (name_len == 3) {
            char a = name[0], b = name[1], c = name[2];
            if (a >= 'a' && a <= 'z') a = (char)(a - 32);
            if (b >= 'a' && b <= 'z') b = (char)(b - 32);
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            if (a == 'B' && b == 'P' && c == 'P') is_bpp = 1;
        }
        // Check "B++"
        if (name_len == 3 &&
            name[0] == 'B' && name[1] == '+' && name[2] == '+')
            is_bpp = 1;
        if (name_len == 3 &&
            name[0] == 'b' && name[1] == '+' && name[2] == '+')
            is_bpp = 1;
        // Check "BASIC", "BASIC+", "BASIC++" (case-insensitive)
        if (name_len >= 5 && name_len <= 7) {
            char u[8];
            int k;
            for (k = 0; k < name_len && k < 7; k++) {
                u[k] = name[k];
                if (u[k] >= 'a' && u[k] <= 'z')
                    u[k] = (char)(u[k] - 32);
            }
            u[k] = '\0';
            if (strcmp(u, "BASIC") == 0 ||
                strcmp(u, "BASIC+") == 0 ||
                strcmp(u, "BASIC++") == 0)
                is_bpp = 1;
        }
        if (is_bpp) {
            dialect_set_strict(0);
            return BASICPP_DEFAULT_DIALECT;
        }
    }

    // Search registered dialects by short code and full name
    for (i = 0; i < DIALECT_COUNT; i++) {
        const char *dn;
        const char *sn;
        int dn_len, sn_len, j;

        if (dialect_table[i].name == NULL) continue;

        dn = dialect_table[i].name;
        sn = dialect_table[i].short_name;
        dn_len = (int)strlen(dn);
        sn_len = (int)strlen(sn);

        // Check short_name exact match first (case-insensitive)
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

        // Check full name substring match (case-insensitive)
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

// -----------------------------------------------------------------
// Dialect Application (Runtime Overrides)
// -----------------------------------------------------------------

// dialect_apply - Apply dialect-specific overrides to the runtime.
//
// Called after dialect_init() to reconfigure the runtime for the
// selected dialect. Two things happen:
//
//   1. The dialect's apply_fn callback is called (if registered).
//      This lets each dialect set runtime flags, adjust RND seed
//      behavior, configure precision modes, etc.
//
//   2. Device aliases for the dialect are loaded. Dialects with
//      platform-specific device names (Atari E:, C64 DEV0:,
//      CoCo CAS:, etc.) auto-load mappings from legacy device
//      names to modern VDev names (CON:, ERR:, FILE:).
//
//   Alias loading is opt-in per dialect:
//     Strong device identity (Atari, C64, CoCo, Sinclair,
//       SuperBASIC, SUPER BASIC): auto-loaded
//     Modern/generic (GW-BASIC, QBasic, ECMA-116): auto-loaded
//       (SCRN:, KYBD: are useful)
//     Minimal (PATB, TRS-80 L1, ECMA-55): no aliases
//
//   Users can override aliases at any time via DEVICE ALIAS commands.
//
void dialect_apply(void)
{
    if (active_dialect && active_dialect->apply_fn) {
        active_dialect->apply_fn();
    }

    // Load device aliases for the active dialect
    if (active_dialect) {
        device_alias_load_dialect(active_dialect->id);
    }
}

// -----------------------------------------------------------------
// Keyword Filtering Modes
// -----------------------------------------------------------------
//
// Three modes control which keywords the parser accepts:
//
//   DMODE_UNION  (0) -- All keywords from all dialects. Default.
//     Every registered keyword is accepted regardless of which
//     dialect is active. This is the "BASIC++" experience.
//
//   DMODE_STRICT (1) -- Only keywords from the active dialect.
//     Keywords whose DFLAG bitmask does not include the active
//     dialect's flag are rejected with "SORRY? ... is not
//     available in this dialect."
//
//   DMODE_MIXED  (2) -- Only keywords from a user-specified subset.
//     The user provides a comma-separated list of dialect codes
//     (e.g., "GWBS,QBAS,C64B") which is compiled into a bitmask.
//     Only keywords matching that composite mask are accepted.
//     Program-mode only; auto-clears at program end.
//
// How to change the default mode:
//   The default is DMODE_UNION (all keywords available).
//   To default to strict mode, call dialect_set_strict(1) during
//   boot in boot.c, or set it via basicpp.cfg.
//
// -----------------------------------------------------------------

static DialectMode dialect_mode = DMODE_UNION;
static DialectMode previous_mode = DMODE_UNION;
static unsigned int mixed_mask = 0;

// dialect_check_feature - Gate a feature flag in strict mode.
//
// In strict mode, if the given flag is 0 (feature not available
// in the active dialect), prints a message and returns 0 (denied).
// In union mode, always returns 1 (allowed).
//
// Parameters:
//   name     - human-readable feature name (for error message)
//   flag     - the dialect config flag to check (0 = unavailable)
//   line_num - BASIC line number (for error context)
//
// Returns:
//   1 if the feature is allowed
//   0 if denied (strict mode, feature not in dialect)
//
int dialect_check_feature(const char *name, int flag, int line_num)
{
    if (dialect_mode == DMODE_STRICT && !flag) {
        (void)line_num;
        printf("SORRY? %s is not available in this dialect.\n",
            name);
        return 0;
    }
    return 1;
}

// dialect_set_strict - Enable or disable strict mode.
//
// Parameters:
//   on - 1 to enable strict mode, 0 to disable
//
// When enabling strict mode:
//   - Mixed mode is cleared (mixed_mask = 0).
//   - Only keywords matching the active dialect's DFLAG are accepted.
//
// When disabling strict mode:
//   - Reverts to union mode (all keywords accepted).
//
void dialect_set_strict(int on)
{
    if (on) {
        dialect_mode = DMODE_STRICT;
        mixed_mask = 0;
    } else {
        if (dialect_mode == DMODE_STRICT)
            dialect_mode = DMODE_UNION;
    }
}

// dialect_is_strict - Query whether strict mode is active.
//
// Returns:
//   1 if strict mode is on
//   0 otherwise (union or mixed mode)
//
int dialect_is_strict(void)
{
    return (dialect_mode == DMODE_STRICT) ? 1 : 0;
}

// dialect_get_mode - Return the current filtering mode.
//
// Returns:
//   DMODE_UNION (0), DMODE_STRICT (1), or DMODE_MIXED (2)
//
DialectMode dialect_get_mode(void)
{
    return dialect_mode;
}

// dialect_get_flag - Return the active dialect's DFLAG bitmask.
//
// Used by the keyword filtering system to check whether a keyword
// belongs to the active dialect.
//
unsigned int dialect_get_flag(void)
{
    return active_dialect->dialect_flag;
}

// dialect_set_mixed - Enable mixed mode with a composite bitmask.
//
// Saves the current mode so it can be restored by
// dialect_clear_mixed(). Only intended for use during deferred
// (program-mode) execution.
//
// Parameters:
//   mask - composite DFLAG bitmask (ORed from multiple dialects)
//
// BASIC usage:
//   OPTION DIALECT MIXED "GWBS,QBAS,C64B"
//
void dialect_set_mixed(unsigned int mask)
{
    if (dialect_mode != DMODE_MIXED) {
        previous_mode = dialect_mode;
    }
    dialect_mode = DMODE_MIXED;
    mixed_mask = mask;
}

// dialect_clear_mixed - Revert from mixed mode.
//
// Restores the mode that was active before mixed mode was entered.
// If not currently in mixed mode, this is a no-op.
//
void dialect_clear_mixed(void)
{
    if (dialect_mode == DMODE_MIXED) {
        dialect_mode = previous_mode;
        mixed_mask = 0;
    }
}

// dialect_is_mixed - Query whether mixed mode is active.
//
// Returns:
//   1 if mixed mode is on
//   0 otherwise (union or strict mode)
//
int dialect_is_mixed(void)
{
    return (dialect_mode == DMODE_MIXED) ? 1 : 0;
}

// dialect_get_mixed_mask - Return the mixed mode bitmask.
//
// Returns the composite DFLAG bitmask built from the user's
// dialect specification. Only meaningful when mixed mode is active.
//
unsigned int dialect_get_mixed_mask(void)
{
    return mixed_mask;
}

// dialect_build_mask - Parse a comma-separated dialect spec string.
//
// Input:  "GWBS,QBAS,C64B"
// Output: (DFLAG_GWBS | DFLAG_QBAS | DFLAG_C64B)
//
// Each comma/space-separated token is matched against registered
// dialect short_name fields via dialect_find_by_name(). Unrecognized
// tokens are silently skipped.
//
// Parameters:
//   spec - comma-separated dialect codes (e.g., "GWBS,QBAS")
//
// Returns:
//   Composite DFLAG bitmask (0 if no valid dialects found)
//
unsigned int dialect_build_mask(const char *spec)
{
    unsigned int mask = 0;
    char token[16];
    int ti = 0;
    int i;

    if (spec == NULL) return 0;

    for (i = 0; ; i++) {
        char c = spec[i];
        if (c == ',' || c == ' ' || c == '\0') {
            if (ti > 0) {
                int did;
                token[ti] = '\0';
                did = dialect_find_by_name(token);
                if (did >= 0 && did < DIALECT_COUNT &&
                    dialect_table[did].name != NULL) {
                    mask |= dialect_table[did].dialect_flag;
                }
                ti = 0;
            }
            if (c == '\0') break;
        } else {
            if (ti < 15) {
                token[ti++] = c;
            }
        }
    }
    return mask;
}

// dialect_keyword_allowed - Check if a keyword is permitted.
//
// This is the main keyword gating function, called by the parser
// before executing any keyword. It checks the keyword's DFLAG
// bitmask against the current filtering mode:
//
//   Union mode:  always returns 1 (all keywords accepted).
//   Strict mode: keyword must match active dialect's DFLAG.
//   Mixed mode:  keyword must match the composite mixed_mask.
//   DFLAG_ALL:   always returns 1 in all modes.
//
// Parameters:
//   kw - KeywordId to check
//
// Returns:
//   1 if the keyword is permitted
//   0 if rejected (prints error message in strict mode)
//
int dialect_keyword_allowed(KeywordId kw)
{
    unsigned int kw_flags;

    // Union mode: everything is allowed
    if (dialect_mode == DMODE_UNION) return 1;

    // Look up the keyword's dialect flags from the lexer
    kw_flags = lexer_get_keyword_flags(kw);

    // DFLAG_ALL keywords are always allowed (e.g., PRINT, LET, END)
    if (kw_flags == DFLAG_ALL) return 1;

    if (dialect_mode == DMODE_STRICT) {
        // Strict mode: check against the active dialect's flag
        if (kw_flags & active_dialect->dialect_flag)
            return 1;
        // Keyword not in this dialect -- print error
        {
            const char *kname = lexer_keyword_name(kw);
            printf("SORRY? %s is not available in this dialect.\n",
                kname ? kname : "keyword");
        }
        return 0;
    }

    if (dialect_mode == DMODE_MIXED) {
        // Mixed mode: check against the composite bitmask
        return (kw_flags & mixed_mask) ? 1 : 0;
    }

    // Fallback (should not reach here)
    return 1;
}

// -----------------------------------------------------------------
// Dialect Security Defaults
// -----------------------------------------------------------------

// dialect_default_security - Get the recommended default security level.
//
// Minimal dialects (PATB, TRS-80 L1, ECMA-55) default to RESTRICTED
// because they have no SHELL, file I/O, or networking capabilities
// on the original machines. More capable dialects default to STANDARD.
//
// Parameters:
//   id - DialectId to query
//
// Returns:
//   SEC_RESTRICTED (2) for minimal dialects
//   SEC_STANDARD (1) for full-featured dialects
//
int dialect_default_security(DialectId id)
{
    if (id == DIALECT_TINY_BASIC || id == DIALECT_TRS80_L1 || id == DIALECT_ECMA55) {
        return SEC_RESTRICTED;
    }
    return SEC_STANDARD;
}
