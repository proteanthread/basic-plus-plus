/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: device_alias_core.h
 * Subsystem: Pluggable Hardware Device Name Translator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Translates legacy device identifiers (LPT1, CAS:) to virtual devices.
 *
 * 2. WHAT TO EXPECT:
 *    Resolves device mappings dynamically based on current dialect.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Device mapping lists, prefix matching rules.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Virtual device interface signatures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If I/O redirection fails, check device spelling and case.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE DEVICE ALIAS CORE
 * File: device_alias_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_DEVICE_ALIAS_CORE_H
#define BASICPP_STANDALONE_DEVICE_ALIAS_CORE_H

#define MAX_DEVICE_ALIASES 32

// Direction flags
#define DEVALIAS_INPUT   0x01
#define DEVALIAS_OUTPUT  0x02
#define DEVALIAS_BOTH    0x03

typedef struct DeviceAlias {
    char alias[16];
    char target[16];
    int direction;
    int dialect;
    int active;
} DeviceAlias;

typedef struct AliasEntry {
    const char *alias;
    const char *target;
    int direction;
} AliasEntry;

void device_alias_core_init(DeviceAlias *table, int *count, int max_aliases);
int device_alias_core_set(DeviceAlias *table, int *count, int max_aliases, const char *alias, const char *target, int direction, int dialect);
const DeviceAlias *device_alias_core_resolve(const DeviceAlias *table, int count, const char *name);
int device_alias_core_remove(DeviceAlias *table, int *count, const char *alias);
int device_alias_core_set_active(DeviceAlias *table, int count, const char *alias, int active);
int device_alias_core_load_preset(DeviceAlias *table, int *count, int max_aliases, int dialect_id);

#endif // BASICPP_STANDALONE_DEVICE_ALIAS_CORE_H
