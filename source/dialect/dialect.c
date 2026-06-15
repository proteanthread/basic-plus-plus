/*
 * ---
 * BASIC++ Interpreter - dialect.c
 * ---
 *
 * Dialect system implementation - Registry Architecture.
 *
 * DESIGN RATIONALE:
 * All dialect-specific behavior is configured through DialectConfig
 * structs registered at boot time. Each dialect lives in its own
 * source file (dialect_gwbs.c, dialect_c64.c, etc.) and registers
 * itself via dialect_register(). This allows:
 *
 * 1. Contributors maintain individual dialect files independently
 * 2. Forks can add/remove dialects without touching core code
 * 3. Conditional compilation (#ifdef) for embedded builds
 * 4. Dialect-specific overrides and test suites per file
 *
 * The parser and executor are dialect-agnostic - they query this
 * module to determine syntax rules, available features, and
 * compatibility behavior.
 *
 * HOW TO ADD A NEW DIALECT:
 * 1. Add to DialectId enum in dialect.h (before DIALECT_COUNT).
 * 2. Create dialect_xxxx.c with a DialectConfig and register func.
 * 3. Add the register call to dialect_register_all() below.
 * 4. Add dialect_xxxx.c to Makefile DIALECT_SOURCES.
 * 5. No parser changes needed - auto-adapts via config queries.
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "dialect.h"
#include "security.h"
#include "device_alias.h"

/* --- Dialect Registry Table ---
 * Mutable table populated by dialect_register() calls at boot.
 * Slots are indexed by DialectId. Empty slots have name == NULL.
 *
 * Replaced the monolithic static const dialect_configs[]
 * array. Each dialect_*.c file now owns its own DialectConfig.
 */
static DialectConfig dialect_table[DIALECT_COUNT];
static int dialect_table_count = 0;

/* Active dialect pointer */
static const DialectConfig *active_dialect = NULL;

/* --- Registration ---
 */

/*
 * dialect_register - Copy a DialectConfig into the registry.
 */
int dialect_register(const DialectConfig *config)
{
 if (config == NULL) return -1;
 if (config->id < 0 || config->id >= DIALECT_COUNT) return -1;

 memcpy(&dialect_table[config->id], config, sizeof(DialectConfig));
 dialect_table_count++;
 return 0;
}

/*
 * dialect_register_all - Call every per-dialect registration function.
 *
 * Each function is defined in its own dialect_*.c file.
 * To add a new dialect, add the call here and create the file.
 */
void dialect_register_all(void)
{
 /* Clear table */
 memset(dialect_table, 0, sizeof(dialect_table));
 dialect_table_count = 0;

 /*
  * Register each compiled-in dialect.
  *
  * On FreeDOS (BPP_FREEDOS defined), only GW-BASIC and ECMA-116
  * are included to fit within 512K conventional memory.  To add
  * a dialect back, uncomment its call here and also:
  *   - Remove its #ifndef guard in dialect.h
  *   - Add its .c file to the Makefile watcom target
  *   - Verify the binary still fits in memory
  *
  * On Windows/Linux, all 16 dialects are always available.
  */
#ifndef BPP_FREEDOS
 dialect_register_patb();
 dialect_register_trs1();
 dialect_register_trs2();
#endif
 dialect_register_gwbs();
#ifndef BPP_FREEDOS
 dialect_register_ecma55();
#endif
 dialect_register_ecma116();
#ifndef BPP_FREEDOS
 dialect_register_qbasic();
 dialect_register_aint();
 dialect_register_asft();
 dialect_register_atari();
 dialect_register_c64();
 dialect_register_coco();
 dialect_register_mbasic();
 dialect_register_sinclair();
 dialect_register_superbasic();
 dialect_register_sbasic();
#endif
}

/* --- Public API ---
 */

/*
 * dialect_init - Select the active dialect.
 *
 * Validates the dialect ID and sets the active_dialect pointer.
 * Falls back to DIALECT_TINY_BASIC for invalid IDs or unregistered
 * dialects.
 */
void dialect_init(DialectId id)
{
 if (id >= 0 && id < DIALECT_COUNT &&
 dialect_table[id].name != NULL) {
 active_dialect = &dialect_table[id];
 } else {
 /*
  * Fallback: use the compile-time default dialect.
  * Set BASICPP_DEFAULT_DIALECT in config.h to change this.
  * On FreeDOS, PATB is not compiled in, so the fallback
  * must be a dialect that IS registered (GWBS or E116).
  */
 active_dialect = &dialect_table[BASICPP_DEFAULT_DIALECT];
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
 * PATB: "READY" GW-BASIC/QBasic: "Ok"
 * Commodore: "READY." CoCo: "OK"
 * AppleSoft: "]" Apple Integer: ">"
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
 * dialect_list_all - Print all registered dialect names.
 *
 * Shows dialect ID, name, short code, and active indicator.
 * Only shows registered (non-NULL) dialect slots.
 */
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
 /* Show B++ as a virtual dialect (all features) */
 printf("  -: %-30s [%s]%s\n",
 "BASIC++ (all features)",
 "B++",
 (dialect_is_strict() == 0) ? " *" : "");
}

/*
 * dialect_find_by_name - Look up a dialect by name.
 *
 * Case-insensitive substring match against dialect names
 * and short names. Only searches registered dialects.
 * Returns the DialectId, or -1 if not found.
 */
int dialect_find_by_name(const char *name)
{
 int i;
 int name_len;

 if (name == NULL) return -1;

 name_len = (int)strlen(name);
 if (name_len == 0) return -1;

 /*
  * "BPP", "B++", and "BASIC++" are not real machine dialects
  * — they select the default all-features-enabled mode.
  * Maps to the compile-time default dialect (typically GWBS)
  * and disables strict mode.
  */
 {
  int is_bpp = 0;
  /* Check "BPP" (case-insensitive) */
  if (name_len == 3) {
   char a = name[0], b = name[1], c = name[2];
   if (a >= 'a' && a <= 'z') a = (char)(a - 32);
   if (b >= 'a' && b <= 'z') b = (char)(b - 32);
   if (c >= 'a' && c <= 'z') c = (char)(c - 32);
   if (a == 'B' && b == 'P' && c == 'P') is_bpp = 1;
  }
  /* Check "B++" */
  if (name_len == 3 &&
      name[0] == 'B' && name[1] == '+' && name[2] == '+')
   is_bpp = 1;
  if (name_len == 3 &&
      name[0] == 'b' && name[1] == '+' && name[2] == '+')
   is_bpp = 1;
  /* Check "BASIC++" (case-insensitive prefix) */
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

 for (i = 0; i < DIALECT_COUNT; i++) {
 const char *dn;
 const char *sn;
 int dn_len, sn_len, j;

 if (dialect_table[i].name == NULL) continue;

 dn = dialect_table[i].name;
 sn = dialect_table[i].short_name;
 dn_len = (int)strlen(dn);
 sn_len = (int)strlen(sn);

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
 * new dialect. Dispatches to the dialect's apply_fn callback if one
 * is registered. Also loads device aliases for the dialect.
 *
 * DEVICE ALIAS LOADING:
 * Dialects with platform-specific device names (Atari E:, C64 DEV0:,
 * CoCo CAS:, etc.) auto-load their aliases here. The aliases map
 * legacy device names to modern VDev names (CON:, ERR:, FILE:).
 *
 * Aliases are opt-in per dialect:
 * - Strong device identity (Atari, C64, CoCo, Sinclair, SuperBASIC,
 *   SUPER BASIC): auto-load
 * - Modern/generic (GW-BASIC, QBasic, ECMA-116): auto-load
 *   (device names like SCRN: and KYBD: are useful)
 * - Minimal (PATB, TRS-80 L1, ECMA-55): no aliases
 *
 * Users can override at any time via DEVICE ALIAS commands.
 */
void dialect_apply(void)
{
 if (active_dialect && active_dialect->apply_fn) {
  active_dialect->apply_fn();
 }

 /* Load device aliases for the active dialect */
 if (active_dialect) {
  device_alias_load_dialect(active_dialect->id);
 }
}


/* --- Dialect Filtering Mode ---
 * Three modes control which keywords are accepted:
 *
 * DMODE_UNION  (0) - All keywords from all dialects (default).
 * DMODE_STRICT (1) - Only keywords from the active dialect.
 * DMODE_MIXED  (2) - Only keywords from a user-specified subset.
 *                     Program-mode only; auto-clears at program end.
 *
 * The previous_mode field remembers what mode was active before
 * mixed mode was entered, so we can restore it on program end.
 */
static DialectMode dialect_mode = DMODE_UNION;
static DialectMode previous_mode = DMODE_UNION;
static unsigned int mixed_mask = 0;

/*
 * dialect_check_feature - Gate a feature in strict mode.
 *
 * In strict mode, if the given flag is 0 (feature not available
 * in this dialect), prints a SORRY message and returns 0.
 * In union mode or if the flag is nonzero, returns 1 (allowed).
 */
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

/*
 * dialect_set_strict - Enable or disable strict mode.
 *
 * When enabling strict, if mixed mode was active it is cleared.
 * When disabling strict, reverts to union mode.
 */
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

/*
 * dialect_is_strict - Query whether strict mode is active.
 */
int dialect_is_strict(void)
{
    return (dialect_mode == DMODE_STRICT) ? 1 : 0;
}

/*
 * dialect_get_mode - Return the current filtering mode.
 */
DialectMode dialect_get_mode(void)
{
 return dialect_mode;
}

/*
 * dialect_get_flag - Return the active dialect's bitmask.
 */
unsigned int dialect_get_flag(void)
{
 return active_dialect->dialect_flag;
}

/*
 * dialect_set_mixed - Enable mixed mode with a bitmask.
 *
 * Saves the current mode so it can be restored later.
 * Only intended for use during program (deferred) execution.
 */
void dialect_set_mixed(unsigned int mask)
{
 if (dialect_mode != DMODE_MIXED) {
  previous_mode = dialect_mode;
 }
 dialect_mode = DMODE_MIXED;
 mixed_mask = mask;
}

/*
 * dialect_clear_mixed - Revert from mixed mode.
 *
 * Restores the mode that was active before mixed mode
 * was entered. If not currently in mixed mode, no-op.
 */
void dialect_clear_mixed(void)
{
 if (dialect_mode == DMODE_MIXED) {
  dialect_mode = previous_mode;
  mixed_mask = 0;
 }
}

/*
 * dialect_is_mixed - Query whether mixed mode is active.
 */
int dialect_is_mixed(void)
{
 return (dialect_mode == DMODE_MIXED) ? 1 : 0;
}

/*
 * dialect_get_mixed_mask - Return the mixed mode bitmask.
 */
unsigned int dialect_get_mixed_mask(void)
{
 return mixed_mask;
}

/*
 * dialect_build_mask - Parse comma-separated dialect codes.
 *
 * Input: "GWBS,QBAS,C64B"
 * Output: (DFLAG_GWBS | DFLAG_QBAS | DFLAG_C64B)
 *
 * Each token is matched against registered dialect short_name
 * fields (case-insensitive). Unrecognized tokens are skipped.
 */
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

/*
 * dialect_keyword_allowed - Check if keyword is permitted.
 *
 * Union mode:  always returns 1.
 * Strict mode: checks keyword flags against active dialect.
 * Mixed mode:  checks keyword flags against mixed bitmask.
 * DFLAG_ALL keywords are always allowed in all modes.
 */
int dialect_keyword_allowed(KeywordId kw)
{
 unsigned int kw_flags;

 /* Union mode: everything allowed */
 if (dialect_mode == DMODE_UNION) return 1;

 /* Look up keyword's dialect flags */
 kw_flags = lexer_get_keyword_flags(kw);

 /* DFLAG_ALL keywords are always allowed */
 if (kw_flags == DFLAG_ALL) return 1;

 if (dialect_mode == DMODE_STRICT) {
  /* Strict: check against single active dialect */
  if (kw_flags & active_dialect->dialect_flag)
   return 1;
  {
   const char *kname = lexer_keyword_name(kw);
   printf("SORRY? %s is not available in this dialect.\n",
    kname ? kname : "keyword");
  }
  return 0;
 }

 if (dialect_mode == DMODE_MIXED) {
  /* Mixed: check against the composite bitmask */
  return (kw_flags & mixed_mask) ? 1 : 0;
 }

 /* Fallback (should not reach here) */
 return 1;
}

/*
 * dialect_default_security - Get the recommended default security level.
 */
int dialect_default_security(DialectId id)
{
    if (id == DIALECT_TINY_BASIC || id == DIALECT_TRS80_L1 || id == DIALECT_ECMA55) {
        return SEC_RESTRICTED; /* 2 */
    }
    return SEC_STANDARD; /* 1 */
}

