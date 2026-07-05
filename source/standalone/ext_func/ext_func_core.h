/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_func_core.h
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
 * File: ext_func_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_EXT_FUNC_CORE_H
#define BASICPP_STANDALONE_EXT_FUNC_CORE_H

#include "../../value.h"

typedef struct BppExtFuncCore {
    const char  *name;
    int          min_args;
    int          max_args;
    int          ret_type;
    int          safety;
    int          required_level;
    const char  *help_text;
    BValue     (*handler)(BValue *args, int argc, void *rt);
} BppExtFuncCore;

typedef struct ExtFuncCoreSlot {
    BppExtFuncCore desc;
    void          *handle;
    int            occupied;
} ExtFuncCoreSlot;

void ext_func_core_init(ExtFuncCoreSlot *table, int max_size, int *count);
int ext_func_core_find(const ExtFuncCoreSlot *table, int count, const char *name);
int ext_func_core_alloc_slot(ExtFuncCoreSlot *table, int max_size, int *count, const char *name);

#endif // BASICPP_STANDALONE_EXT_FUNC_CORE_H
