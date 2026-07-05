/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: funcreg_core.c
 * Subsystem: Variant Library Functions Dictionary
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers standard library math/string/system function wrappers.
 *
 * 2. WHAT TO EXPECT:
 *    Executes functions by mapping string calls to pointers.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Built-in function tables, alias rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Function parameter signature structure.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If function execution fails, verify parameter data types.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE FUNCTION REGISTRY CORE
 * File: funcreg_core.c
 * ===================================================================== */

#include <string.h>
#include <ctype.h>
#include "funcreg_core.h"

static int fstr_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

void funcreg_core_init(FuncCoreEntry *table, int *count, int max_entries)
{
    if (table && count) {
        memset(table, 0, (size_t)max_entries * sizeof(FuncCoreEntry));
        *count = 0;
    }
}

int funcreg_core_register(FuncCoreEntry *table, int *count, int max_entries, const FuncCoreEntry *entry)
{
    if (!table || !count || !entry || !entry->name) return -1;
    if (*count >= max_entries) return -1;

    // Check for duplicate name
    for (int i = 0; i < *count; i++) {
        if (table[i].name && fstr_iequal(table[i].name, entry->name)) {
            // Already registered, return success (idempotent)
            return 0;
        }
    }

    table[*count] = *entry;
    (*count)++;
    return 0;
}

const FuncCoreEntry *funcreg_core_find_by_keyword(const FuncCoreEntry *table, int count, int kw)
{
    if (!table) return NULL;
    for (int i = 0; i < count; i++) {
        if (table[i].keyword == kw) {
            return &table[i];
        }
    }
    return NULL;
}

const FuncCoreEntry *funcreg_core_find_by_name(const FuncCoreEntry *table, int count, const char *name)
{
    if (!table || !name) return NULL;
    for (int i = 0; i < count; i++) {
        if (table[i].name && fstr_iequal(table[i].name, name)) {
            return &table[i];
        }
    }
    return NULL;
}

int funcreg_core_override(FuncCoreEntry *table, int count, int kw, FuncCoreHandler handler, int security_level_open)
{
    if (!table) return -1;
    for (int i = 0; i < count; i++) {
        if (table[i].keyword == kw) {
            if (!table[i].overridable) {
                return -1; // Not overridable
            }
            if (!security_level_open) {
                return -2; // Locked by security level
            }
            table[i].handler = handler;
            return 0;
        }
    }
    return -3; // Not found
}
