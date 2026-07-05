/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: error_registry_core.c
 * Subsystem: Dynamic Custom Error Messages Registry
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers custom dynamic errors and retrieves user-friendly error strings.
 *
 * 2. WHAT TO EXPECT:
 *    Translates error codes to descriptive messages.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Custom message buffers, localized text pools.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Built-in error code mapping rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If messages do not resolve, check index bounds.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE ERROR REGISTRY CORE
 * File: error_registry_core.c
 * ===================================================================== */

#include <string.h>
#include "error_registry_core.h"

void error_registry_core_init(ErrorCoreEntry *table, int *count, int max_entries)
{
    if (table && count) {
        *count = 0;
        memset(table, 0, (size_t)max_entries * sizeof(ErrorCoreEntry));
    }
}

int error_registry_core_register(ErrorCoreEntry *table, int *count, int max_entries, int error_code, const char *syntax_name)
{
    int i;
    if (!table || !count) return -1;

    // Check for existing entry (update in place)
    for (i = 0; i < *count; i++) {
        if (table[i].error_code == error_code) {
            table[i].syntax_name = syntax_name;
            return 0;
        }
    }

    // Append new entry
    if (*count >= max_entries) return -1;
    table[*count].error_code = error_code;
    table[*count].syntax_name = syntax_name;
    (*count)++;
    return 0;
}

const char *error_registry_core_lookup(const ErrorCoreEntry *table, int count, int error_code)
{
    int i;
    if (table) {
        for (i = 0; i < count; i++) {
            if (table[i].error_code == error_code) {
                return table[i].syntax_name;
            }
        }
    }

    switch (error_code) {
        case 1: return "NEXT without FOR";
        case 2: return "Syntax error";
        case 3: return "RETURN without GOSUB";
        case 4: return "Out of data";
        case 5: return "Illegal function call";
        case 6: return "Overflow";
        case 7: return "Out of memory";
        case 8: return "Undefined line";
        case 9: return "Subscript out of range";
        case 10: return "Redimensioned array";
        case 11: return "Division by zero";
        case 13: return "Type mismatch";
        case 14: return "Out of string space";
        case 15: return "String too long";
        case 16: return "String formula too complex";
        case 17: return "Can't continue";
        case 18: return "Undefined user function";
        default: return "Unknown Error";
    }
}
