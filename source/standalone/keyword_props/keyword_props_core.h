/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: keyword_props_core.h
 * Subsystem: Keyword Dynamic Behavior Overrides
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Maintains keyword flags (e.g. UPPERCASE) to modify parser rules.
 *
 * 2. WHAT TO EXPECT:
 *    Modifies syntax validation on the fly.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default property flags, property definitions.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Property lookup rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If override fails to apply, verify dialect validation flags.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE KEYWORD PROPERTY CORE
 * File: keyword_props_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_KEYWORD_PROPS_CORE_H
#define BASICPP_STANDALONE_KEYWORD_PROPS_CORE_H

#define MAX_KW_PROPS 8
#define MAX_PROP_NAME 32
#define MAX_PROP_VALUE 64

typedef struct {
    char name[MAX_PROP_NAME];
    char value[MAX_PROP_VALUE];
} KwProp;

typedef struct {
    KwProp props[MAX_KW_PROPS];
    int count;
} KwPropSet;

void keyword_props_core_init(KwPropSet *table, int num_kws);
int keyword_props_core_set(KwPropSet *table, int num_kws, int kw, const char *name, const char *value);
const char *keyword_props_core_get(const KwPropSet *table, int num_kws, int kw, const char *name);
int keyword_props_core_is_on(const KwPropSet *table, int num_kws, int kw, const char *name);
int keyword_props_core_get_int(const KwPropSet *table, int num_kws, int kw, const char *name, int default_val);
void keyword_props_core_remove(KwPropSet *table, int num_kws, int kw, const char *name);

#endif // BASICPP_STANDALONE_KEYWORD_PROPS_CORE_H
