/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: override_core.h
 * Subsystem: Keyword Redirection Overrides Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Registers and tracks alias redirections for keywords.
 *
 * 2. WHAT TO EXPECT:
 *    Updates parser bindings to redirect calls.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default overrides lists.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Redirection target structures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If circular loop occurs, clear overrides database.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE KEYWORD OVERRIDE CORE
 * File: override_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_OVERRIDE_CORE_H
#define BASICPP_STANDALONE_OVERRIDE_CORE_H

typedef struct OverrideCoreEntry {
    char text[128];
    int active;
} OverrideCoreEntry;

void override_core_init(OverrideCoreEntry *table, int num_ids);
int override_core_set(OverrideCoreEntry *table, int num_ids, int id, const char *text, int is_protected);
const char *override_core_get(const OverrideCoreEntry *table, int num_ids, int id);
void override_core_clear(OverrideCoreEntry *table, int num_ids, int id);

#endif // BASICPP_STANDALONE_OVERRIDE_CORE_H
