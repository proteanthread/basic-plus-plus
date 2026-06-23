/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Plugin manager implementing support for loadable dynamic modules and external extensions.
 *    - Dynamic library resolution, plugin loading, and runtime command callbacks.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "gw_plugin.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define MAX_PLUGINS 16

struct GW_PluginManager {
    void *handles[MAX_PLUGINS];
    char paths[MAX_PLUGINS][256];
};

GW_PluginManager *gw_plugin_init(void) {
    GW_PluginManager *pm = (GW_PluginManager *)malloc(sizeof(GW_PluginManager));
    if (!pm) return NULL;
    
    for (int i = 0; i < MAX_PLUGINS; i++) {
        pm->handles[i] = NULL;
        pm->paths[i][0] = '\0';
    }
    
    return pm;
}

void gw_plugin_cleanup(GW_PluginManager *pm) {
    if (!pm) return;
    
    for (int i = 0; i < MAX_PLUGINS; i++) {
        if (pm->handles[i]) {
            gw_plugin_unload(pm, i);
        }
    }
    
    free(pm);
}

int gw_plugin_load(GW_PluginManager *pm, const char *path) {
    if (!pm || !path) return -1;
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_PLUGINS; i++) {
        if (!pm->handles[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) return -1;
    
    void *handle = NULL;
#ifdef _WIN32
    handle = (void *)LoadLibraryA(path);
#else
    handle = dlopen(path, RTLD_LAZY);
#endif
    
    if (!handle) return -1;
    
    pm->handles[slot] = handle;
    strncpy(pm->paths[slot], path, 255);
    pm->paths[slot][255] = '\0';
    
    return slot;
}

void gw_plugin_unload(GW_PluginManager *pm, int handle) {
    if (!pm || handle < 0 || handle >= MAX_PLUGINS || !pm->handles[handle]) return;
    
#ifdef _WIN32
    FreeLibrary((HMODULE)pm->handles[handle]);
#else
    dlclose(pm->handles[handle]);
#endif
    
    pm->handles[handle] = NULL;
    pm->paths[handle][0] = '\0';
}

int gw_plugin_call(GW_PluginManager *pm, int handle, const char *func_name, int num_args, void **args, int *arg_types) {
    if (!pm || handle < 0 || handle >= MAX_PLUGINS || !pm->handles[handle] || !func_name) return -1;
    
    GW_PluginFunc func = NULL;
#ifdef _WIN32
    func = (GW_PluginFunc)GetProcAddress((HMODULE)pm->handles[handle], func_name);
#else
    func = (GW_PluginFunc)dlsym(pm->handles[handle], func_name);
#endif
    
    if (!func) return -1;
    
    func(num_args, args, arg_types);
    return 0;
}
