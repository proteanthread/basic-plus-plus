/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: funcreg_core.h
 * Subsystem: Variant Library Functions Dictionary
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers standard library math/string/system function wrappers.
 *
 * 2. WHAT TO EXPECT:
 *    Executes functions by mapping string calls to pointers.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Built-in function tables, alias rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Function parameter signature structure.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If function execution fails, verify parameter data types.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE FUNCTION REGISTRY CORE
 * File: funcreg_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_FUNCREG_CORE_H
#define BASICPP_STANDALONE_FUNCREG_CORE_H

#define MAX_FUNCTIONS 128

typedef enum FuncCoreCategory {
    FCAT_CORE_VAL = 0,
    FCAT_MATH_VAL = 1,
    FCAT_STRING_VAL = 2,
    FCAT_IO_VAL = 3,
    FCAT_UTIL_VAL = 4,
    FCAT_USER_VAL = 5
} FuncCoreCategory;

typedef enum FuncCoreReturnType {
    FRET_INT_VAL = 0,
    FRET_FLOAT_VAL = 1,
    FRET_STRING_VAL = 2,
    FRET_ANY_VAL = 3
} FuncCoreReturnType;

typedef enum FuncCoreSafety {
    FSAFE_PURE_VAL = 0,
    FSAFE_STATE_VAL = 1,
    FSAFE_IO_VAL = 2,
    FSAFE_SYSTEM_VAL = 3
} FuncCoreSafety;

typedef void (*FuncCoreHandler)(void);

typedef struct FuncCoreEntry {
    const char *name;
    int keyword;
    int category;
    int ret_type;
    int min_args;
    int max_args;
    int safety;
    int overridable;
    FuncCoreHandler handler;
    const char *help_text;
} FuncCoreEntry;

void funcreg_core_init(FuncCoreEntry *table, int *count, int max_entries);
int funcreg_core_register(FuncCoreEntry *table, int *count, int max_entries, const FuncCoreEntry *entry);
const FuncCoreEntry *funcreg_core_find_by_keyword(const FuncCoreEntry *table, int count, int kw);
const FuncCoreEntry *funcreg_core_find_by_name(const FuncCoreEntry *table, int count, const char *name);
int funcreg_core_override(FuncCoreEntry *table, int count, int kw, FuncCoreHandler handler, int security_level_open);

#endif // BASICPP_STANDALONE_FUNCREG_CORE_H
