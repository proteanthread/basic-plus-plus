/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_feature_core.c
 * Subsystem: Extended Capabilities Plugin Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers extended system features such as serial ports or network drivers.
 *
 * 2. WHAT TO EXPECT:
 *    Maintains feature tables and validates requirements.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Available features, feature limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Feature descriptor signatures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If features fail to load, check plugin location and permissions.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE EXTERNAL FEATURE REGISTRY CORE
 * File: ext_feature_core.c
 * ===================================================================== */

#include "ext_feature_core.h"
#include <stddef.h>

static int feat_str_iequal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void ext_feature_core_init(BppExtFeatureCore *table, int max_features, int *count)
{
    if (table && count) {
        int i;
        for (i = 0; i < max_features; i++) {
            table[i].name[0] = '\0';
            table[i].version[0] = '\0';
            table[i].spec_path[0] = '\0';
            table[i].lib_path[0] = '\0';
            table[i].required_level = 0;
            table[i].spec_loaded = 0;
            table[i].lib_loaded = 0;
            table[i].keyword_count = 0;
        }
        *count = 0;
    }
}

int ext_feature_core_find(const BppExtFeatureCore *table, int count, const char *name)
{
    if (!table || !name) return -1;
    {
        int i;
        for (i = 0; i < count; i++) {
            if ((table[i].spec_loaded || table[i].lib_loaded) && feat_str_iequal(table[i].name, name)) {
                return i;
            }
        }
    }
    return -1;
}

int ext_feature_core_alloc_slot(BppExtFeatureCore *table, int max_features, int *count, const char *name)
{
    if (!table || !count || !name) return -1;

    // Check if name is already loaded
    {
        int existing = ext_feature_core_find(table, *count, name);
        if (existing >= 0) {
            return existing;
        }
    }

    // Try to find a free slot
    {
        int i;
        for (i = 0; i < *count; i++) {
            if (!table[i].spec_loaded && !table[i].lib_loaded) {
                return i;
            }
        }
    }

    // Allocate next slot
    if (*count >= max_features) {
        return -1;
    }

    {
        int allocated = *count;
        (*count)++;
        return allocated;
    }
}
