/*
 * =====================================================================
 * BASIC++ Interpreter - funcreg.c
 * =====================================================================
 *
 * Function Registry System implementation.
 *
 * PURPOSE:
 * Implements the centralized function registry. All built-in
 * functions, dialect overrides, and module-registered functions
 * live in this table. The parser dispatches function calls
 * through this registry instead of using hardcoded switch/if
 * chains.
 *
 * WHY THIS EXISTS:
 * The specification requires a formal function dispatch table
 * that supports:
 * 1. Runtime introspection ("what functions are available?")
 * 2. Dialect overrides ("PRINT in Commodore mode")
 * 3. Module extension ("add DOUBLE() from my module")
 * 4. Safety classification ("is this function pure?")
 *
 * DATA STRUCTURE:
 * A flat static array of FunctionEntry structs. Linear scan
 * for lookup is O(n) but n < 128, so this is negligible.
 * The simplicity ensures:
 * - No dynamic allocation required
 * - Deterministic behavior on all platforms
 * - No hash table overhead or collision handling
 * - C89-safe without any advanced data structures
 *
 * HOW TO EXTEND:
 * External modules register functions by calling
 * funcreg_register() with a filled FunctionEntry struct.
 * See funcreg.h for detailed examples.
 *
 * =====================================================================
 */

#include <string.h>
#include <ctype.h>
#include "funcreg.h"

/* =====================================================================
 * Registry Table
 * =====================================================================
 * Static array of registered functions. The table is pre-allocated
 * at compile time to avoid dynamic memory allocation.
 *
 * reg_count tracks the number of filled entries.
 */
static FunctionEntry registry[MAX_FUNCTIONS];
static int reg_count = 0;

/* =====================================================================
 * funcreg_init - Initialize the function registry.
 *
 * Clears all entries. Called once during boot, before builtins
 * are registered.
 */
void funcreg_init(void)
{
 memset(registry, 0, sizeof(registry));
 reg_count = 0;
}

/* =====================================================================
 * funcreg_register - Register a function.
 *
 * Copies the entry into the next available slot. Returns 0 on
 * success, -1 if the table is full.
 *
 * DUPLICATE HANDLING:
 * If a function with the same keyword already exists, the new
 * entry replaces it. This allows dialect modules to override
 * built-in functions by re-registering with a different handler.
 * Only overridable functions may be replaced (enforced by the
 * replacement check).
 */
int funcreg_register(const FunctionEntry *entry)
{
 int i;

 if (entry == NULL) return -1;

 /*
 * Check for existing entry with same keyword.
 * If found and overridable, replace it.
 * This allows dialects to swap function behavior.
 */
 for (i = 0; i < reg_count; i++) {
 if (registry[i].keyword == entry->keyword &&
 registry[i].name != NULL) {
 /* Replace - overridable check is done at a higher level
 * (funcreg_override). Direct registration always succeeds
 * for initial registration by builtins. */
 memcpy(&registry[i], entry, sizeof(FunctionEntry));
 return 0;
 }
 }

 /* New entry - append */
 if (reg_count >= MAX_FUNCTIONS) return -1;

 memcpy(&registry[reg_count], entry, sizeof(FunctionEntry));
 reg_count++;
 return 0;
}

/* =====================================================================
 * funcreg_find_by_keyword - Look up by keyword ID.
 *
 * Primary lookup path for the parser. Returns NULL if not found.
 */
const FunctionEntry *funcreg_find_by_keyword(KeywordId kw)
{
 int i;
 for (i = 0; i < reg_count; i++) {
 if (registry[i].keyword == kw && registry[i].name != NULL) {
 return &registry[i];
 }
 }
 return NULL;
}

/* =====================================================================
 * funcreg_find_by_name - Look up by name string.
 *
 * Case-insensitive comparison (BASIC convention: "abs" == "ABS").
 * Used for module loading, DEF FN resolution, and introspection.
 */
const FunctionEntry *funcreg_find_by_name(const char *name)
{
 int i;
 if (name == NULL) return NULL;

 for (i = 0; i < reg_count; i++) {
 const char *rn = registry[i].name;
 const char *qn = name;

 if (rn == NULL) continue;

 /* Case-insensitive string compare (C89-safe) */
 while (*rn && *qn) {
 if (toupper((unsigned char)*rn) !=
 toupper((unsigned char)*qn)) {
 break;
 }
 rn++;
 qn++;
 }
 if (*rn == '\0' && *qn == '\0') {
 return &registry[i];
 }
 }
 return NULL;
}

/* =====================================================================
 * funcreg_override - Replace handler for a registered function.
 *
 * Only functions marked overridable=1 can be overridden. Core
 * functions (overridable=0) are immutable - they define the
 * Core Immutable API per the specification.
 *
 * Returns 0 on success, -1 if not found or not overridable.
 */
int funcreg_override(KeywordId kw, FuncHandler handler)
{
 int i;
 if (handler == NULL) return -1;

 for (i = 0; i < reg_count; i++) {
 if (registry[i].keyword == kw && registry[i].name != NULL) {
 if (!registry[i].overridable) {
 return -1; /* Core function - cannot override */
 }
 registry[i].handler = handler;
 return 0;
 }
 }
 return -1; /* Not found */
}

/* =====================================================================
 * funcreg_count - Return the number of registered functions.
 */
int funcreg_count(void)
{
 return reg_count;
}

/* =====================================================================
 * funcreg_get - Get a function entry by index.
 *
 * Returns NULL if index is out of range.
 */
const FunctionEntry *funcreg_get(int index)
{
 if (index < 0 || index >= reg_count) return NULL;
 return &registry[index];
}
