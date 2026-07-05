/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_plugin_core.c
 * Subsystem: Yaml Plugin Manifest Parser
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses dynamic YAML plugins descriptors and registers features.
 *
 * 2. WHAT TO EXPECT:
 *    Reads plugin properties and maps dependencies.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Allowed plugin features list.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    YAML parser constraints.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If plugin registration fails, verify plugin manifest file syntax.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE EXTERNAL PLUGIN REGISTRY CORE
 * File: ext_plugin_core.c
 * ===================================================================== */

#include "ext_plugin_core.h"
#include <stddef.h>

static int plug_str_iequal(const char *a, const char *b)
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

void ext_plugin_core_init(BppPluginCore *table, int max_plugins, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_plugins; i++) {
            table[i].name[0] = '\0';
            table[i].version[0] = '\0';
            table[i].author[0] = '\0';
            table[i].description[0] = '\0';
            table[i].path[0] = '\0';
            table[i].required_level = 0;
            table[i].entry_count = 0;
            table[i].loaded = 0;
        }
        *count = 0;
    }
}

int ext_plugin_core_find(const BppPluginCore *table, int count, const char *name)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].loaded && plug_str_iequal(table[i].name, name)) {
                return i;
            }
        }
    }
    return -1;
}

int ext_plugin_core_alloc_slot(BppPluginCore *table, int max_plugins, int *count, const char *name)
{
    if (!table || !count || !name) return -1;

    // Check if name is already loaded
    {
        int existing = ext_plugin_core_find(table, *count, name);
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
    if (*count >= max_plugins) {
        return -1;
    }

    {
        int allocated = *count;
        (*count)++;
        return allocated;
    }
}
