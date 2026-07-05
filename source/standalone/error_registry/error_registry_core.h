/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: error_registry_core.h
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
 * File: error_registry_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_ERROR_REGISTRY_CORE_H
#define BASICPP_STANDALONE_ERROR_REGISTRY_CORE_H

typedef struct ErrorCoreEntry {
    int error_code;
    const char *syntax_name;
} ErrorCoreEntry;

void error_registry_core_init(ErrorCoreEntry *table, int *count, int max_entries);
int error_registry_core_register(ErrorCoreEntry *table, int *count, int max_entries, int error_code, const char *syntax_name);
const char *error_registry_core_lookup(const ErrorCoreEntry *table, int count, int error_code);

#endif // BASICPP_STANDALONE_ERROR_REGISTRY_CORE_H
