/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: dialect.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    BASIC historical dialect configuration profiles and strict execution filters.
 *
 * 2. WHAT TO EXPECT:
 *    Dialect configurations switch prompt layouts, printing zone widths, statement separators, and active parsing tokens.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Dialect parameters (READY prompt, separator char), addition of custom dialects.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Dialect lookup logic, strict-mode keyword mask validation.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check dialect switches. If syntax is rejected, check if STRICT mode is enabled or keyword is filtered out.
 * ===================================================================== */

 // dialect.h -- dialect system interface
 //
 // Every BASIC dialect has its own syntax quirks: PATB uses ';'
 // between statements while everyone else uses ':', some require
 // LET, some don't, prompts differ, line number ranges differ.
 // This header defines the configuration struct and API that lets
 // the parser adapt to any dialect without per-dialect if/else.
 //
 // Adding a new machine-specific dialect:
 //   1. Pick a 4-char code. Add DIALECT_xxx to DialectId below.
 //   2. Add DFLAG_xxx (next available bit in the bitmask).
 //   3. Create dialect_xxx.c with a DialectConfig struct and a
 //      dialect_register_xxx() function.
 //   4. Wire it into dialect_register_all() in dialect.c.
 //   5. Add dialect_xxx.c to the Makefile.
 //   6. Add machine-specific keywords to lexer.c with your DFLAG.
 //   That's it. The parser picks up the new config automatically.
 //
 // Creating a custom (non-machine-specific) dialect:
 //   You can define a dialect that doesn't map to any real hardware.
 //   Just set the flags to whatever combination makes sense. For
 //   example, a "BASIC++ Modern" dialect could enable WHILE/WEND,
 //   DO/LOOP, ON ERROR, extended variable names, and every other
 //   feature simultaneously. Set the prompt to whatever you want.
 //   Custom dialects are useful for building domain-specific
 //   languages on top of the BASIC++ parser.
 //
 // Mixing dialects:
 //   By default (OPTION STRICT OFF), all keywords from all dialects
 //   are available simultaneously. A program can use WHILE (GWBS)
 //   alongside TRAP (Atari) alongside CLR (C64). To restrict to
 //   a single dialect, use OPTION STRICT ON -- then only keywords
 //   tagged with the active dialect's DFLAG are accepted.
 //   DIALECT switching at runtime is fully supported.
 //
 // Memory pools auto-adjust:
 //   DialectConfig doesn't set memory sizes. Pool sizing is handled
 //   in runtime.c (rt_init) and config.h. If you need a dialect with
 //   different memory limits, override the limits after dialect_init()
 //   by writing a dialect-specific apply function. The pools grow
 //   dynamically when possible (string pool, array storage).
 //
 // Using BASIC++ to build other language interpreters:
 //   The parser-lexer-runtime stack is generic enough that you can
 //   redefine keywords to create a completely new language. Swap the
 //   keyword table in lexer.c, write new parse handlers in parser.c,
 //   and you have a different language running on the same VM. The
 //   codegen backend (codegen.c) can transpile to C, so your new
 //   language gets a free native compiler.
 //
 // Detokenizing older formats:
 //   GW-BASIC and C64 saved programs in binary tokenized format
 //   (first byte 0xFF for GW-BASIC, various for Commodore). A
 //   detokenizer reads these binary files and converts to ASCII
 //   text that BASIC++ can LOAD. The token-to-keyword mapping
 //   lives in lexer.c's keyword table. See bytecode.h for the
 //   serialization API -- bpp_load() is where you'd add format
 //   detection and automatic detokenization.
 //
 // Self-modifying code:
 //   BASIC++ stores programs as text lines indexed by line number.
 //   A running program can modify itself: build a string containing
 //   a BASIC line and feed it through exec_line(). The next time
 //   that line number is reached, the new code runs. This is how
 //   some 1980s copy protection worked.
 //
 // Transpiling to other targets:
 //   codegen.c currently emits C89. To target Python or Pascal,
 //   write a new AST walker (look at codegen_emit_stmt). The AST
 //   nodes in ast.h are language-neutral -- IF/FOR/WHILE/GOSUB
 //   map naturally to any imperative language. For Pascal, emit
 //   BEGIN/END blocks instead of braces. For Python, track indent
 //   level and emit colons.

#ifndef BASICPP_DIALECT_H
#define BASICPP_DIALECT_H

#include "lexer.h" // KeywordId

// --- Dialect Bitmask Flags ---
 // Each keyword is tagged with a bitmask indicating which dialects
 // support it. In union mode (default) all keywords are accepted.
 // In strict mode (OPTION STRICT) only keywords whose bitmask
 // includes the active dialect are allowed.
 //
 // 16-bit unsigned: one bit per dialect (15 used, 1 reserved).
#define DFLAG_PATB (1u << 0) // Palo Alto Tiny BASIC
#define DFLAG_TRS1 (1u << 1) // TRS-80 Level I
#define DFLAG_TRS2 (1u << 2) // TRS-80 Level II
#define DFLAG_GWBS (1u << 3) // GW-BASIC
#define DFLAG_EC55 (1u << 4) // ECMA-55
#define DFLAG_E116 (1u << 5) // ECMA-116
#define DFLAG_QBAS (1u << 6) // QBasic
#define DFLAG_AINT (1u << 7) // Apple II Integer BASIC
#define DFLAG_ASFT (1u << 8) // AppleSoft BASIC
#define DFLAG_ATRI (1u << 9) // Atari/Microsoft BASIC II
#define DFLAG_C64B (1u << 10) // Commodore BASIC v2
#define DFLAG_COCO (1u << 11) // Tandy CoCo BASIC
#define DFLAG_MBAS (1u << 12) // Microsoft MBASIC (CP/M)
#define DFLAG_SINC (1u << 13) // Sinclair BASIC (ZX Spectrum)
#define DFLAG_SUPA (1u << 14) // SuperBASIC (Sinclair QL)
#define DFLAG_SBAS (1u << 15) // SUPER BASIC (Tymshare)
#define DFLAG_ALL (0xFFFFu) // all dialects / BASIC++ native

// Convenience: Microsoft BASIC family (shared core)
#define DFLAG_MSBASIC (DFLAG_TRS2 | DFLAG_GWBS | DFLAG_QBAS | \
 DFLAG_ASFT | DFLAG_C64B | DFLAG_COCO | DFLAG_MBAS)

// Convenience: All Microsoft + both TRS-80 levels
#define DFLAG_MSALL (DFLAG_TRS1 | DFLAG_MSBASIC)

// Convenience: Structured BASIC (WHILE/WEND, SELECT, etc.)
#define DFLAG_STRUCT (DFLAG_GWBS | DFLAG_QBAS | DFLAG_E116 | DFLAG_MBAS | \
 DFLAG_SUPA)

// Convenience: GW-BASIC + QBasic (most compatible pair)
#define DFLAG_GWQB (DFLAG_GWBS | DFLAG_QBAS)


// --- Dialect Identifiers ---
 // Each supported BASIC dialect has a unique identifier.
 // DIALECT_TINY_BASIC is the default and the only fully
 // implemented dialect in 
typedef enum DialectId {
 DIALECT_TINY_BASIC = 0, // Palo Alto Tiny BASIC (default)
 DIALECT_TRS80_L1, // TRS-80 Level I BASIC
 DIALECT_TRS80_L2, // TRS-80 Level II BASIC
 DIALECT_GW_BASIC, // GW-BASIC
 DIALECT_ECMA55, // ECMA-55 Minimal BASIC
 DIALECT_ECMA116, // ECMA-116 Full BASIC
 DIALECT_QBASIC, // QBasic (subset)
 DIALECT_APPLE_INT, // Apple II Integer BASIC
 DIALECT_APPLESOFT, // AppleSoft BASIC
 DIALECT_ATARI_MS, // Atari/Microsoft BASIC II
 DIALECT_COMMODORE, // Commodore BASIC
 DIALECT_COCO, // Tandy Color Computer BASIC
 DIALECT_MBASIC, // Microsoft MBASIC (CP/M)
 DIALECT_SINCLAIR, // Sinclair BASIC (ZX Spectrum 48K)
 DIALECT_SUPERBASIC, // SuperBASIC (Sinclair QL)
 DIALECT_SBASIC, // SUPER BASIC (Tymshare SDS-940)
 DIALECT_COUNT // sentinel - must be last
} DialectId;

// --- Dialect Configuration ---
 // Contains all dialect-specific flags and settings. The parser
 // and executor read these flags to adjust their behavior.
 //
 // Fields:
 // id - dialect identifier
 // name - human-readable name (for display)
 // stmt_separator - character separating statements on one
 // line (';' for PATB, ':' for most others)
 // has_then_keyword - 1 if IF requires THEN, 0 if not (PATB: 0)
 // has_let_optional - 1 if LET keyword can be omitted (A=5)
 // has_for_next - 1 if FOR/NEXT loops are supported
 // has_string_vars - 1 if string variables (A$) are supported
 // has_print_hash - 1 if PRINT # format specifier is supported
 // has_array_at - 1 if @() array syntax is supported
 // has_rnd_function - 1 if RND() function is available
 // has_abs_function - 1 if ABS() function is available
 // has_size_function - 1 if SIZE function is available
 // max_line_number - maximum valid line number
 // not_eq_is_hash - 1 if # is the not-equal operator
typedef struct DialectConfig {
 DialectId id;
 const char *name;
 char stmt_separator;
 int has_then_keyword;
 int has_let_optional;
 int has_for_next;
 int has_string_vars;
 int has_print_hash;
 int has_array_at;
 int has_rnd_function;
 int has_abs_function;
 int has_size_function;
 int max_line_number;
 int not_eq_is_hash;
 // additions
 int has_data_read; // DATA/READ/RESTORE support
 int has_while_wend; // WHILE/WEND loops
 int has_do_loop; // DO/LOOP loops
 int has_extended_vars; // named variables beyond A-Z
 int has_merge_chain; // MERGE/CHAIN commands
 // additions
 int has_float; // floating-point arithmetic
 int has_dim_arrays; // DIM arrays
 int has_string_functions; // string functions
 // additions - dialect personality
 const char *ready_prompt; // ready prompt text
 int print_zone_width; // PRINT zone width (tab stops)
 int has_on_error; // ON ERROR GOTO support
 int has_cls; // CLS command
 int has_tron_troff; // TRON/TROFF support
 const char *short_name; // 4-char dialect code for DIALECT$
 unsigned int dialect_flag; // bitmask flag for this dialect
 // Dialect-specific apply callback. Called after dialect_init()
  // to configure runtime behavior unique to this dialect.
  // NULL = no special configuration needed. 
 void (*apply_fn)(void);
} DialectConfig;

 // dialect_check_feature - Gate a feature in strict mode.
 //
 // In strict mode, checks if the given flag is enabled for
 // the active dialect. Returns 1 if allowed, 0 if blocked
 // (and prints a SORRY message). In union mode, always returns 1.
 //
 // Usage:
 //   if (!dialect_check_feature("floating point", dc->has_float, ln))
 //       return;
int dialect_check_feature(const char *name, int flag, int line_num);

// --- Dialect Functions ---

 // dialect_init - Set the active dialect.
 //
 // Configures the interpreter to use the specified dialect's rules.
 // Must be called once at startup (default: DIALECT_TINY_BASIC).
 // Can be called again to switch dialects at runtime.
void dialect_init(DialectId id);

 // dialect_get_config - Return the active dialect configuration.
 //
 // Returns a pointer to the current DialectConfig. The returned
 // pointer is valid until dialect_init() is called again.
const DialectConfig *dialect_get_config(void);

 // dialect_get_name - Return the display name of the active dialect.
const char *dialect_get_name(void);

 // dialect_get_separator - Return the statement separator character.
 //
 // Returns ';' for PATB, ':' for most other dialects.
char dialect_get_separator(void);

 // dialect_list_all - Print all available dialect names and IDs.
 //
 // Used by the DIALECT command without arguments to show
 // available options.
void dialect_list_all(void);

 // dialect_find_by_name - Look up a dialect by name (case-insensitive).
 //
 // Returns the DialectId, or -1 if not found.
int dialect_find_by_name(const char *name);

 // dialect_get_ready_prompt - Return the dialect-specific ready prompt.
 //
 // Different BASICs used different prompts:
 // PATB: "READY" GW-BASIC: "Ok" Commodore: "READY."
const char *dialect_get_ready_prompt(void);

 // dialect_get_zone_width - Return PRINT zone width.
 //
 // Classic BASICs used different tab column widths for comma-
 // separated PRINT items. Typical values: 8, 14, 16.
int dialect_get_zone_width(void);

 // dialect_get_short_name - Return 4-8 character dialect code.
 //
 // Used by DIALECT$ introspection. E.g., "PATB", "TRS1", "GWBS".
 // "BPP" (or "B++") is the sole 3-char exception: selects
 // the all-features-enabled mode (no strict filtering).
const char *dialect_get_short_name(void);

 // dialect_apply - Apply dialect-specific overrides.
 //
 // Called after dialect_init() to apply function registry overrides
 // and other dialect-specific runtime configuration. This is where
 // the function registry integration happens.
 //
 // Reconfigures function availability based on dialect
 // feature flags (e.g., disabling string functions for PATB).
void dialect_apply(void);

 // dialect_set_strict - Enable/disable strict dialect mode.
 //
 // When strict mode is ON, keywords that don't belong to the active
 // dialect's bitmask are rejected (raise WHAT?).
 // When OFF (default), all keywords from all dialects are accepted.
 //
 // Activated via: OPTION STRICT (enable)
 // dialect_set_strict - Set the strict mode flag.
 //
 // 1 = strict mode (only active dialect keywords), 0 = union mode.
void dialect_set_strict(int strict);

 // dialect_is_strict - Query whether strict mode is active.
int dialect_is_strict(void);

 // dialect_default_security - Get the recommended default security level.
 //
 // Minimalist dialects run securely under RESTRICTED, while heavy
 // systems request STANDARD or OPEN.
int dialect_default_security(DialectId id);

int dialect_is_strict(void);

 // dialect_keyword_allowed - Check if a keyword is allowed.
 //
 // In union mode:  always returns 1.
 // In strict mode: returns 1 only if the keyword's dialect bitmask
 //                 includes the active dialect's flag.
 // In mixed mode:  returns 1 only if the keyword's dialect bitmask
 //                 intersects the mixed-mode mask.
 //
 // Keywords tagged DFLAG_ALL are always allowed in all modes.
 //
 // The keyword's dialect flags are looked up from the lexer's
 // keyword table via lexer_get_keyword_flags().
int dialect_keyword_allowed(KeywordId kw);

 // dialect_get_flag - Return the active dialect's bitmask flag.
unsigned int dialect_get_flag(void);

// --- Dialect Mode Enumeration ---
 // Three keyword filtering modes:
 //
 // DMODE_UNION  - All keywords from all dialects accepted (default).
 //                This is the only mode available in immediate mode
 //                besides strict.
 //
 // DMODE_STRICT - Only keywords from the single active dialect are
 //                accepted. Available in both immediate and deferred
 //                (program) mode.
 //
 // DMODE_MIXED  - Only keywords from a user-specified subset of
 //                dialects are accepted. ONLY available in deferred
 //                (program) mode. Auto-clears when the program ends
 //                (RUN completes, STOP, END, or error), reverting
 //                to the previous mode.
typedef enum DialectMode {
    DMODE_UNION = 0,
    DMODE_STRICT = 1,
    DMODE_MIXED = 2
} DialectMode;

 // dialect_get_mode - Return the current dialect filtering mode.
DialectMode dialect_get_mode(void);

 // dialect_set_mixed - Enable mixed mode with a bitmask.
 //
 // Sets the dialect filtering mode to DMODE_MIXED and stores the
 // given bitmask. Only keywords whose dialect flags intersect this
 // mask (or are DFLAG_ALL) are accepted.
 //
 // Mixed mode is program-only: it should be activated by OPTION MIXED
 // within a running program. When the program ends, call
 // dialect_clear_mixed() to revert.
 //
 // Parameters:
 //   mask - OR'd combination of DFLAG_xxx values
 //          e.g., (DFLAG_GWBS | DFLAG_QBAS)
void dialect_set_mixed(unsigned int mask);

 // dialect_clear_mixed - Clear mixed mode and revert.
 //
 // If currently in DMODE_MIXED, reverts to DMODE_UNION (or
 // DMODE_STRICT if strict was active before mixed was set).
 // Called automatically at program end.
void dialect_clear_mixed(void);

 // dialect_is_mixed - Query whether mixed mode is active.
int dialect_is_mixed(void);

 // dialect_get_mixed_mask - Return the current mixed mode bitmask.
 //
 // Returns 0 if not in mixed mode.
unsigned int dialect_get_mixed_mask(void);

 // dialect_build_mask - Build a bitmask from comma-separated codes.
 //
 // Parses a string like "GWBS,QBAS,C64B" and returns the OR'd
 // bitmask of all matched dialects. Returns 0 if none matched.
 //
 // Parameters:
 //   spec - comma-separated dialect short codes
unsigned int dialect_build_mask(const char *spec);

// --- Dialect Registration (- Contributor Architecture) ---
 // Each dialect lives in its own source file (dialect_gwbs.c, etc.)
 // and registers itself via dialect_register() at boot time.
 //
 // This allows contributors to maintain individual dialect files
 // independently and enables conditional compilation (#ifdef) to
 // include or exclude specific dialects for embedded builds.

 // dialect_register - Register a dialect configuration.
 //
 // Called by each dialect_*.c file during dialect_register_all().
 // The config is copied into the internal table at slot config->id.
 // Returns 0 on success, -1 if id is out of range.
int dialect_register(const DialectConfig *config);

 // dialect_register_all - Register all compiled-in dialects.
 //
 // Called once from main.c during boot, before dialect_init().
 // Calls each dialect_register_XXXX() function.
void dialect_register_all(void);

// Per-dialect registration functions (one per dialect_*.c file)
 //
 // On FreeDOS builds (BPP_FREEDOS defined), only GW-BASIC and
 // ECMA-116 are compiled in.  The remaining declarations are
 // guarded to avoid unresolved symbol errors at link time.
 //
 // To add a dialect back to the FreeDOS build:
 // 1. Remove the #ifndef guard around its declaration here.
 // 2. Uncomment its register call in dialect_register_all()
 //    in dialect.c.
 // 3. Add its .c file back to the Makefile watcom target.
 // 4. Verify the binary still fits within 512K.
#ifndef BPP_FREEDOS
void dialect_register_patb(void);
void dialect_register_trs1(void);
void dialect_register_trs2(void);
#endif
void dialect_register_gwbs(void);
#ifndef BPP_FREEDOS
void dialect_register_ecma55(void);
#endif
void dialect_register_ecma116(void);
#ifndef BPP_FREEDOS
void dialect_register_qbasic(void);
void dialect_register_aint(void);
void dialect_register_asft(void);
void dialect_register_atari(void);
void dialect_register_c64(void);
void dialect_register_coco(void);
void dialect_register_mbasic(void);
void dialect_register_sinclair(void);
void dialect_register_superbasic(void);
void dialect_register_sbasic(void);
#endif

#endif // BASICPP_DIALECT_H
