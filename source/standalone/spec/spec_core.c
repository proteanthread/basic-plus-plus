/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: spec_core.c
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
 * File: spec_core.c
 * ===================================================================== */

#include "spec_core.h"
#include <string.h>

void spec_core_init(SpecObjectCore *registry, int max_specs, int *count)
{
    if (registry && count) {
        int i;
        for (i = 0; i < max_specs; i++) {
            registry[i].name[0] = '\0';
            registry[i].version[0] = '\0';
            registry[i].category = 0;
            registry[i].depends[0] = '\0';
            registry[i].lib_path[0] = '\0';
            registry[i].kw_id = 0;
            registry[i].required_level = 0;
        }
        *count = 0;
    }
}

int spec_core_find(const SpecObjectCore *registry, int count, const char *name)
{
    if (!registry || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (strcmp(registry[i].name, name) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int spec_core_find_by_kw(const SpecObjectCore *registry, int count, int kw_id)
{
    if (!registry) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if (registry[i].kw_id == kw_id) {
                return i;
            }
        }
    }
    return -1;
}

int spec_core_add(SpecObjectCore *registry, int max_specs, int *count, const SpecObjectCore *spec)
{
    if (!registry || !count || !spec) return -1;
    if (*count >= max_specs) return -1;
    {
        int idx = *count;
        registry[idx] = *spec;
        (*count)++;
        return idx;
    }
}
