/*
 * =====================================================================
 * BASIC++ Interpreter - dialect.c
 * =====================================================================
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
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "dialect.h"

/* =====================================================================
 * Dialect Registry Table
 * =====================================================================
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

/* =====================================================================
 * Registration
 * =====================================================================
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

 /* Register each compiled-in dialect */
 dialect_register_patb();
 dialect_register_trs1();
 dialect_register_trs2();
 dialect_register_gwbs();
 dialect_register_ecma55();
 dialect_register_ecma116();
 dialect_register_qbasic();
 dialect_register_aint();
 dialect_register_asft();
 dialect_register_atari();
 dialect_register_c64();
 dialect_register_coco();
}

/* =====================================================================
 * Public API
 * =====================================================================
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
 active_dialect = &dialect_table[DIALECT_TINY_BASIC];
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
 * new dialect. Future: each dialect_*.c can provide its own
 * apply function for dialect-specific behavior (e.g., Commodore
 * RND(0), Atari CLR semantics).
 */
void dialect_apply(void)
{
 /* Placeholder for dialect-specific overrides.
 * Future dialect_*.c files can register override callbacks. */
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
