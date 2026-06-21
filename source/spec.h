/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: spec.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

#ifndef BASICPP_SPEC_H
#define BASICPP_SPEC_H

#include "lexer.h"
#include "security.h"

// --- Specification Categories ---
typedef enum {
    SPEC_CAT_UNKNOWN = 0,
    SPEC_CAT_STATEMENT,
    SPEC_CAT_FUNCTION,
    SPEC_CAT_DIALECT,
    SPEC_CAT_MODULE,
    SPEC_CAT_LIBRARY,
    SPEC_CAT_DEVICE
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
} SpecCategory;

// --- Specification Object ---
typedef struct SpecObject {
    char name[64];
    char version[16];
    SpecCategory category;
    char depends[256];
    char lib_path[256]; // Path to the external .LIB file for behavior
    KeywordId kw_id; // If it defines a new statement/function
    SecLevel required_level; // Pinned security level (SEC_COUNT = unpinned)
} SpecObject;

// --- Specification Registry API ---
#define MAX_SPECS 128

void spec_registry_init(void);
int spec_load_file(const char *filename);
SpecObject* spec_find(const char *name);
void spec_list_all(void);

#endif // BASICPP_SPEC_H
