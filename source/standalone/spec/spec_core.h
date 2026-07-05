/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: spec_core.h
 * Subsystem: Custom Statements Dispatch Bindings
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers custom statements parser and executor bindings.
 *
 * 2. WHAT TO EXPECT:
 *    Dynamic extensions of parser grammar.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Spec attributes layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Dispatch table mappings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If custom keyword fails to run, check spec registration.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE SPECIFICATION REGISTRY CORE
 * File: spec_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_SPEC_CORE_H
#define BASICPP_STANDALONE_SPEC_CORE_H

typedef enum {
    SPEC_CAT_CORE_UNKNOWN = 0,
    SPEC_CAT_CORE_STATEMENT,
    SPEC_CAT_CORE_FUNCTION,
    SPEC_CAT_CORE_DIALECT,
    SPEC_CAT_CORE_MODULE,
    SPEC_CAT_CORE_LIBRARY,
    SPEC_CAT_CORE_DEVICE
} SpecCategoryCore;

typedef struct SpecObjectCore {
    char name[64];
    char version[16];
    int  category;
    char depends[256];
    char lib_path[256];
    int  kw_id;
    int  required_level;
} SpecObjectCore;

void spec_core_init(SpecObjectCore *registry, int max_specs, int *count);
int spec_core_find(const SpecObjectCore *registry, int count, const char *name);
int spec_core_find_by_kw(const SpecObjectCore *registry, int count, int kw_id);
int spec_core_add(SpecObjectCore *registry, int max_specs, int *count, const SpecObjectCore *spec);

#endif // BASICPP_STANDALONE_SPEC_CORE_H
