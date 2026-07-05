/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: module_core.c
 * Subsystem: Dynamic Features Registry Handler
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers and tracks dynamically loaded system modules.
 *
 * 2. WHAT TO EXPECT:
 *    Coordinates startup/shutdown of standard modules.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default modules table.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module calling interfaces.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If module registration fails, check dependencies.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE MODULE REGISTRY CORE
 * File: module_core.c
 * ===================================================================== */

#include "module_core.h"
#include <stddef.h>

static int str_iequal(const char *a, const char *b)
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

void module_core_init(ModuleCoreSlot *table, int max_modules, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_modules; i++) {
            table[i].active = 0;
            table[i].occupied = 0;
            table[i].info.name = NULL;
            table[i].info.version = NULL;
            table[i].info.description = NULL;
            table[i].info.mod_class = 0;
            table[i].info.capabilities = 0;
            table[i].info.init = NULL;
            table[i].info.cleanup = NULL;
        }
        *count = 0;
    }
}

int module_core_register(ModuleCoreSlot *table, int max_modules, int *count, const ModuleCoreInfo *info)
{
    if (!table || !count || !info || !info->name) return -1;

    if (*count >= max_modules) {
        return -1;
    }

    // Check for duplicate name
    {
        int i;
        for (i = 0; i < *count; i++) {
            if (table[i].occupied && str_iequal(table[i].info.name, info->name)) {
                // Already registered
                return 0;
            }
        }
    }

    table[*count].info = *info;
    table[*count].active = 0;
    table[*count].occupied = 1;
    (*count)++;

    return 0;
}

const ModuleCoreInfo *module_core_find(const ModuleCoreSlot *table, int count, const char *name, int *out_index)
{
    if (!table || !name) return NULL;

    {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].occupied && str_iequal(table[i].info.name, name)) {
                if (out_index) *out_index = i;
                return &table[i].info;
            }
        }
    }

    return NULL;
}

int module_core_activate(ModuleCoreSlot *table, int index, void *rt)
{
    if (!table || index < 0) return -1;
    if (!table[index].occupied) return -1;

    if (table[index].active) {
        return 0;
    }

    if (table[index].info.init) {
        int res = table[index].info.init(rt);
        if (res != 0) return -1;
    }

    table[index].active = 1;
    return 0;
}

int module_core_deactivate(ModuleCoreSlot *table, int index)
{
    if (!table || index < 0) return -1;
    if (!table[index].occupied || !table[index].active) return -1;

    if (table[index].info.cleanup) {
        table[index].info.cleanup();
    }

    table[index].active = 0;
    return 0;
}
