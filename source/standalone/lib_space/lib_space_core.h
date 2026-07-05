/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lib_space_core.h
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
 * File: lib_space_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_LIB_SPACE_CORE_H
#define BASICPP_STANDALONE_LIB_SPACE_CORE_H

#include "../../pcode.h"
#include "../../value.h"

#define MAX_LOADED_LIBS     16
#define MAX_LIB_SYMBOLS     64
#define MAX_LIB_NAME        64
#define MAX_LIB_NAMED_VARS  64

typedef enum LibSymbolTypeCore {
    LIB_SYM_CORE_SUB = 0,
    LIB_SYM_CORE_FUNCTION,
    LIB_SYM_CORE_DEF_FN
} LibSymbolTypeCore;

typedef struct LibSymbolCore {
    char            name[MAX_LIB_NAME];
    int             type;
    int             entry_offset;
    int             param_count;
    int             active;
} LibSymbolCore;

typedef struct LibNamedVarCore {
    char    name[MAX_LIB_NAME];
    BValue  val;
    int     is_string;
    int     active;
} LibNamedVarCore;

typedef enum LibExtTypeCore {
    LIB_EXT_CORE_LIBRARY = 0,
    LIB_EXT_CORE_FUNCTION,
    LIB_EXT_CORE_FEATURE,
    LIB_EXT_CORE_MODULE,
    LIB_EXT_CORE_PLUGIN
} LibExtTypeCore;

typedef struct LibSourceLineCore {
    char    text[256];
    int     vline;
} LibSourceLineCore;

typedef struct LoadedLibraryCore {
    char            name[MAX_LIB_NAME];
    char            version[16];
    char            path[256];
    int             ext_type;
    int             required_level;
    PCodeProgram    pcode;
    LibSourceLineCore *src_lines;
    int             src_line_count;
    int             src_line_cap;
    LibSymbolCore   symbols[MAX_LIB_SYMBOLS];
    int             symbol_count;
    double          num_vars[26];
    char           *str_vars[26];
    LibNamedVarCore named_vars[MAX_LIB_NAMED_VARS];
    int             named_var_count;
    int             loaded;
    int             compiled;
} LoadedLibraryCore;

void lib_space_core_init(LoadedLibraryCore *table, int max_libs, int *count);
int lib_space_core_find(const LoadedLibraryCore *table, int count, const char *name);
int lib_space_core_find_symbol(const LoadedLibraryCore *table, int count, const char *name, int *out_lib_idx);
int lib_space_core_alloc_slot(const LoadedLibraryCore *table, int max_libs, int count);

#endif // BASICPP_STANDALONE_LIB_SPACE_CORE_H
