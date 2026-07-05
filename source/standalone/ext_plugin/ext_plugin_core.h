/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_plugin_core.h
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
 * File: ext_plugin_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_EXT_PLUGIN_CORE_H
#define BASICPP_STANDALONE_EXT_PLUGIN_CORE_H

#define BPP_PLUG_MODULE   0
#define BPP_PLUG_LIBRARY  1
#define BPP_PLUG_FEATURE  2
#define BPP_PLUG_SPEC     3

#define MAX_PLUGIN_ENTRIES 16

typedef struct BppPluginEntryCore {
    int          type;
    char         filename[256];
} BppPluginEntryCore;

typedef struct BppPluginCore {
    char         name[64];
    char         version[16];
    char         author[64];
    char         description[256];
    char         path[256];
    int          required_level;
    int          entry_count;
    BppPluginEntryCore entries[MAX_PLUGIN_ENTRIES];
    int          loaded;
} BppPluginCore;

void ext_plugin_core_init(BppPluginCore *table, int max_plugins, int *count);
int ext_plugin_core_find(const BppPluginCore *table, int count, const char *name);
int ext_plugin_core_alloc_slot(BppPluginCore *table, int max_plugins, int *count, const char *name);

#endif // BASICPP_STANDALONE_EXT_PLUGIN_CORE_H
