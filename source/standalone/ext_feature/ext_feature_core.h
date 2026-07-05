/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ext_feature_core.h
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
 * File: ext_feature_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_EXT_FEATURE_CORE_H
#define BASICPP_STANDALONE_EXT_FEATURE_CORE_H

typedef struct BppExtFeatureCore {
    char         name[64];
    char         version[16];
    char         spec_path[256];
    char         lib_path[256];
    int          required_level;
    int          spec_loaded;
    int          lib_loaded;
    int          keyword_count;
} BppExtFeatureCore;

void ext_feature_core_init(BppExtFeatureCore *table, int max_features, int *count);
int ext_feature_core_find(const BppExtFeatureCore *table, int count, const char *name);
int ext_feature_core_alloc_slot(BppExtFeatureCore *table, int max_features, int *count, const char *name);

#endif // BASICPP_STANDALONE_EXT_FEATURE_CORE_H
