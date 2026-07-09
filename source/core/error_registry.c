/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: error_registry.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - error_registry.c
// ---
//
// Extensible error code architecture.
//
// PURPOSE:
//   Maps numeric error codes (GW-BASIC/QBasic compatible) to
//   human-readable error message strings. Provides both built-in
//   error messages (PATB/MS-BASIC standard codes 1-18) and a
//   registry for custom error codes added by modules or plugins.
//
// HOW IT WORKS:
//   The error registry has two layers:
//
//   Layer 1 -- Custom Registry (error_map):
//     A dynamic table of (error_code -> syntax_name) pairs.
//     Modules and plugins register custom error codes here
//     via error_registry_register(). Looked up first.
//
//   Layer 2 -- Built-in Fallbacks:
//     A hardcoded switch statement mapping GW-BASIC/QBasic
//     standard error codes (1-18) to their classic messages.
//     Used if the custom registry has no match.
//
//   The ERR$() BASIC function uses error_registry_lookup() to
//   convert an error code to its message string.
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new built-in error code:
//   1. Add a case to the switch statement in error_registry_lookup().
//   2. Use the GW-BASIC error code number if there's a standard one.
//   3. For custom codes, use numbers >= 100 to avoid conflicts.
//
//   Registering custom error codes from a module:
//     error_registry_register(100, "Custom error description");
//   This can be called from any module's init or register function.
//
//   Overriding a built-in error message:
//     error_registry_register(11, "MY DIVISION BY ZERO");
//   Custom registry entries take priority over built-in fallbacks.
//
// FINE-TUNING:
//   - MAX_CUSTOM_ERRORS (128): Maximum custom error entries.
//     Increase if you need more custom error codes.
//   - Error code numbering follows GW-BASIC convention (1-18).
//     Codes 19-99 are reserved for future built-in errors.
//     Codes >= 100 are available for custom use.
//
// TROUBLESHOOTING:
//   - ERR$() returns "Unknown Error":
//     The error code is not registered. Either add it to the
//     switch fallback or register it via error_registry_register().
//   - "Registry full" (returns -1):
//     MAX_CUSTOM_ERRORS limit reached. Increase the constant.
//
// PERFORMANCE:
//   - error_registry_lookup(): O(N) linear scan (N = custom count).
//     Fallback switch is O(1). Total: fast for typical use.
//
// MINIMALIZATION:
//   This file is CORE tier -- required for ERR$() support.
//   For a minimal build without ERR$(): remove this file and
//   stub out error_registry_lookup() to return "Error".
//
// DEPENDENCIES:
//   - error_registry.h (function declarations)
//   - value.h          (BValue for ERR$() builtin)
//   - runtime.h        (RuntimeState for last_err_code)
//   - stringpool.h     (strpool_alloc for string result)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (string.h).
//   - No platform-specific code.
//
// ---

#include "error_registry.h"
#include "value.h"
#include "runtime.h"
#include "stringpool.h"
#include <string.h>

// Maximum number of custom error registrations.
// Built-in fallback errors (1-18) do not count toward this limit.
#define MAX_CUSTOM_ERRORS 128

// Custom error entry: maps an error code to a message string.
typedef struct {
    int error_code;
    const char *syntax_name;  // pointer to static or module-owned string
} ErrorEntry;

// Custom error registry -- populated by modules/plugins.
static ErrorEntry error_map[MAX_CUSTOM_ERRORS];
static int error_count = 0;

// error_registry_init - Clear the custom error registry.
//
// Called during boot (Phase 4). Does NOT affect the built-in
// fallback error messages -- those are hardcoded.
//
void error_registry_init(void) {
    error_count = 0;
    memset(error_map, 0, sizeof(error_map));
}

// error_registry_register - Register or update a custom error code.
//
// If the error code already exists, updates the message.
// If not, appends a new entry.
//
// Parameters:
//   error_code  - numeric error code (recommend >= 100 for custom)
//   syntax_name - human-readable error message (must remain valid)
//
// Returns:
//   0 on success
//  -1 if the registry is full
//
int error_registry_register(int error_code, const char *syntax_name) {
    int i;
    // Check for existing entry (update in place)
    for (i = 0; i < error_count; i++) {
        if (error_map[i].error_code == error_code) {
            error_map[i].syntax_name = syntax_name;
            return 0;
        }
    }
    // Append new entry
    if (error_count >= MAX_CUSTOM_ERRORS) return -1;
    error_map[error_count].error_code = error_code;
    error_map[error_count].syntax_name = syntax_name;
    error_count++;
    return 0;
}

// error_registry_lookup - Look up an error code's message string.
//
// Search order:
//   1. Custom registry (linear scan)
//   2. Built-in GW-BASIC/QBasic error codes (switch)
//   3. "Unknown Error" fallback
//
// Parameters:
//   error_code - numeric error code to look up
//
// Returns:
//   Pointer to the error message string (static or registered).
//   Never returns NULL.
//
// GW-BASIC/QBasic error code reference:
//    1: NEXT without FOR      2: Syntax error
//    3: RETURN without GOSUB  4: Out of data
//    5: Illegal function call 6: Overflow
//    7: Out of memory         8: Undefined line
//    9: Subscript out of range 10: Redimensioned array
//   11: Division by zero     13: Type mismatch
//   14: Out of string space  15: String too long
//   16: String formula too complex
//   17: Can't continue       18: Undefined user function
//
const char *error_registry_lookup(int error_code) {
    int i;
    // Check custom registry first
    for (i = 0; i < error_count; i++) {
        if (error_map[i].error_code == error_code) {
            return error_map[i].syntax_name;
        }
    }
    // Fallbacks for native PATB / MS-BASIC common errors
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
        case 80: return "Assertion failed";
    }
    return "Unknown Error";
}

// builtin_err_str - ERR$() built-in function implementation.
//
// Returns the error message string for a given error code.
// If called with no arguments, uses the last error code from
// the runtime state.
//
// BASIC usage:
//   PRINT ERR$(11)    -> "Division by zero"
//   PRINT ERR$()      -> message for the last error
//
// Parameters:
//   args  - BValue argument array
//   argc  - argument count (0 or 1)
//   rt    - RuntimeState pointer (void* to avoid circular deps)
//
// Returns:
//   BValue string containing the error message.
//
BValue builtin_err_str(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt;
    int err_code;
    const char *err_str;
    char *buf;
    int len;

    if (argc < 1) {
        err_code = state->last_err_code;  // ERR$() with no args
    } else {
        err_code = (int)bval_to_int(&args[0]);
    }

    err_str = error_registry_lookup(err_code);
    if (!err_str) err_str = "Unknown Error";

    len = (int)strlen(err_str);
    buf = strpool_alloc(&state->strpool, len);
    if (buf == NULL) return bval_string(NULL, 0);
    memcpy(buf, err_str, (size_t)len);
    return bval_string(buf, len);
}
