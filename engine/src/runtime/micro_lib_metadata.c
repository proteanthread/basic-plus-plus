/* =====================================================================
 * What it does: Implements the Micro-Library Embedded Metadata Registry.
 * Why it exists: Manages the lookup table for HELP, CATALOG, and Error Codes for all 1-to-1 micro-libraries.
 * Why it works this way: Stores pointers to static metadata structs registered by individual micro-libraries.
 * What can be changed: Search algorithm or array allocation.
 * What cannot be changed: Case-insensitive string matching logic.
 * What to expect: Zero allocation overhead since string literals are stored in code memory.
 * What to do if something breaks: Ensure microlib_init is called during phase 1 boot.
 * Assumptions: Strings passed in MicroLibMetadata are valid for the lifetime of the process.
 * Portability concerns: Strict C17 compliance.
 * Future expansions: Indexing by category for CATALOG fast grouping.
 * ===================================================================== */

#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>

static MicroLibMetadata g_registry[MAX_MICROLIB_ENTRIES];
static int g_count = 0;

void microlib_init(void) {
    memset(g_registry, 0, sizeof(g_registry));
    g_count = 0;
}

int microlib_register(const MicroLibMetadata *meta) {
    if (!meta || !meta->name) return -1;
    if (g_count >= MAX_MICROLIB_ENTRIES) return -1;

    /* Check for duplicate registration */
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].name && strcmp(g_registry[i].name, meta->name) == 0) {
            g_registry[i] = *meta;
            return i;
        }
    }

    g_registry[g_count] = *meta;
    return g_count++;
}

static int strcasecmp_portable(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = toupper((unsigned char)*s1);
        int c2 = toupper((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

const MicroLibMetadata *microlib_find(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].name && strcasecmp_portable(g_registry[i].name, name) == 0) {
            return &g_registry[i];
        }
    }
    return NULL;
}

int microlib_count(void) {
    return g_count;
}

const MicroLibMetadata *microlib_get(int index) {
    if (index < 0 || index >= g_count) return NULL;
    return &g_registry[index];
}
