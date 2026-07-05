/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_lib_core.c
 * Subsystem: Dynamic Library Linker Wrapper
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Interfaces with host dynamic linking loader (dlopen/LoadLibrary).
 *
 * 2. WHAT TO EXPECT:
 *    Loads files, links functions, returns module handles.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Search path list, file extensions search loops.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Platform-specific loader APIs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If library cannot load, check platform bitness and path permissions.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE EXTERNAL LIBRARY REGISTRY CORE
 * File: ext_lib_core.c
 * ===================================================================== */

#include "ext_lib_core.h"
#include <stddef.h>

static int lib_str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void ext_lib_core_init(BppExtLibCore *table, int max_libs, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_libs; i++) {
            table[i].name[0] = '\0';
            table[i].version[0] = '\0';
            table[i].path[0] = '\0';
            table[i].required_level = 0;
            table[i].format = 0;
            table[i].func_count = 0;
            table[i].loaded = 0;
        }
        *count = 0;
    }
}

int ext_lib_core_find(const BppExtLibCore *table, int count, const char *name)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].loaded && lib_str_iequal(table[i].name, name)) {
                return i;
            }
        }
    }
    return -1;
}

int ext_lib_core_alloc_slot(BppExtLibCore *table, int max_libs, int *count, const char *name)
{
    if (!table || !count || !name) return -1;

    // Check if name is already loaded
    {
        int existing = ext_lib_core_find(table, *count, name);
        if (existing >= 0) {
            return existing;
        }
    }

    // Try to find a free slot
    {
        int i;
        for (i = 0; i < *count; i++) {
            if (!table[i].loaded) {
                return i;
            }
        }
    }

    // Allocate next slot
    if (*count >= max_libs) {
        return -1;
    }

    {
        int allocated = *count;
        (*count)++;
        return allocated;
    }
}
