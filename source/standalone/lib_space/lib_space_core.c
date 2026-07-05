/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lib_space_core.c
 * Subsystem: Isolated Module Library Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages compiled libraries slots and symbol lookups.
 *
 * 2. WHAT TO EXPECT:
 *    Validates and maps calls inside dynamic code segments.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Maximum slots limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Library code protection boundaries.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If modules do not load, verify compile status.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE LIBRARY SPACE REGISTRY CORE
 * File: lib_space_core.c
 * ===================================================================== */

#include "lib_space_core.h"
#include <string.h>

static int ls_str_iequal(const char *a, const char *b)
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

void lib_space_core_init(LoadedLibraryCore *table, int max_libs, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_libs; i++) {
            table[i].name[0] = '\0';
            table[i].version[0] = '\0';
            table[i].path[0] = '\0';
            table[i].ext_type = 0;
            table[i].required_level = 0;
            table[i].pcode.instrs = NULL;
            table[i].pcode.count = 0;
            table[i].pcode.capacity = 0;
            table[i].src_lines = NULL;
            table[i].src_line_count = 0;
            table[i].src_line_cap = 0;
            table[i].symbol_count = 0;
            table[i].named_var_count = 0;
            table[i].loaded = 0;
            table[i].compiled = 0;
        }
        *count = 0;
    }
}

int lib_space_core_find(const LoadedLibraryCore *table, int count, const char *name)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].loaded && ls_str_iequal(table[i].name, name)) {
                return i;
            }
        }
    }
    return -1;
}

int lib_space_core_find_symbol(const LoadedLibraryCore *table, int count, const char *name, int *out_lib_idx)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (!table[i].loaded) continue;
            {
                int j;
                for (j = 0; j < table[i].symbol_count; j++) {
                    if (table[i].symbols[j].active && ls_str_iequal(table[i].symbols[j].name, name)) {
                        if (out_lib_idx) *out_lib_idx = i;
                        return j;
                    }
                }
            }
        }
    }
    if (out_lib_idx) *out_lib_idx = -1;
    return -1;
}

int lib_space_core_alloc_slot(const LoadedLibraryCore *table, int max_libs, int count)
{
    if (!table) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (!table[i].loaded) return i;
        }
    }
    if (count < max_libs) {
        return count;
    }
    return -1;
}
