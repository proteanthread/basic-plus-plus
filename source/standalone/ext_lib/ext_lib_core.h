/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_lib_core.h
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
 * File: ext_lib_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_EXT_LIB_CORE_H
#define BASICPP_STANDALONE_EXT_LIB_CORE_H

typedef struct BppExtLibCore {
    char         name[64];
    char         version[16];
    char         path[256];
    int          required_level;
    int          format;
    int          func_count;
    int          loaded;
} BppExtLibCore;

void ext_lib_core_init(BppExtLibCore *table, int max_libs, int *count);
int ext_lib_core_find(const BppExtLibCore *table, int count, const char *name);
int ext_lib_core_alloc_slot(BppExtLibCore *table, int max_libs, int *count, const char *name);

#endif // BASICPP_STANDALONE_EXT_LIB_CORE_H
