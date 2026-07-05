/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_func_core.c
 * Subsystem: External Dynamic Library Function Bindings
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Binds and executes compiled C functions from dynamic libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Converts parameters and invokes external entry points.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Maximum arguments count limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Standard calling convention rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If execution fails, verify function names and calling convention.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE EXTERNAL FUNCTION REGISTRY CORE
 * File: ext_func_core.c
 * ===================================================================== */

#include "ext_func_core.h"
#include <stddef.h>

static int ext_str_iequal(const char *a, const char *b)
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

void ext_func_core_init(ExtFuncCoreSlot *table, int max_size, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_size; i++) {
            table[i].occupied = 0;
            table[i].handle = NULL;
            table[i].desc.name = NULL;
            table[i].desc.min_args = 0;
            table[i].desc.max_args = 0;
            table[i].desc.ret_type = 0;
            table[i].desc.safety = 0;
            table[i].desc.required_level = 0;
            table[i].desc.help_text = NULL;
            table[i].desc.handler = NULL;
        }
        *count = 0;
    }
}

int ext_func_core_find(const ExtFuncCoreSlot *table, int count, const char *name)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].occupied && ext_str_iequal(table[i].desc.name, name)) {
                return i;
            }
        }
    }
    return -1;
}

int ext_func_core_alloc_slot(ExtFuncCoreSlot *table, int max_size, int *count, const char *name)
{
    if (!table || !count || !name) return -1;

    // Check if name is already registered (hot-reload target)
    {
        int existing = ext_func_core_find(table, *count, name);
        if (existing >= 0) {
            return existing;
        }
    }

    // Try to find a free slot among existing count
    {
        int i;
        for (i = 0; i < *count; i++) {
            if (!table[i].occupied) {
                return i;
            }
        }
    }

    // Check if table is full
    if (*count >= max_size) {
        return -1;
    }

    // Allocate next slot
    {
        int allocated = *count;
        (*count)++;
        return allocated;
    }
}
