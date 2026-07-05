/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: module_core.h
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
 * File: module_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_MODULE_CORE_H
#define BASICPP_STANDALONE_MODULE_CORE_H

typedef struct ModuleCoreInfo {
    const char *name;
    const char *version;
    const char *description;
    int         mod_class;
    unsigned int capabilities;
    int (*init)(void *);
    void (*cleanup)(void);
} ModuleCoreInfo;

typedef struct ModuleCoreSlot {
    ModuleCoreInfo info;
    int active;
    int occupied;
} ModuleCoreSlot;

void module_core_init(ModuleCoreSlot *table, int max_modules, int *count);
int module_core_register(ModuleCoreSlot *table, int max_modules, int *count, const ModuleCoreInfo *info);
const ModuleCoreInfo *module_core_find(const ModuleCoreSlot *table, int count, const char *name, int *out_index);
int module_core_activate(ModuleCoreSlot *table, int index, void *rt);
int module_core_deactivate(ModuleCoreSlot *table, int index);

#endif // BASICPP_STANDALONE_MODULE_CORE_H
