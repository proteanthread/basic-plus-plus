/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: override_core.c
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
 * File: override_core.c
 * ===================================================================== */

#include <string.h>
#include "override_core.h"

void override_core_init(OverrideCoreEntry *table, int num_ids)
{
    if (table) {
        int i;
        for (i = 0; i < num_ids; i++) {
            table[i].active = 0;
            table[i].text[0] = '\0';
        }
    }
}

int override_core_set(OverrideCoreEntry *table, int num_ids, int id, const char *text, int is_protected)
{
    int tlen;

    if (!table || id < 0 || id >= num_ids) {
        return -1;
    }

    if (is_protected) {
        return -1;
    }

    if (text == NULL || text[0] == '\0') {
        return -1;
    }

    tlen = (int)strlen(text);
    if (tlen >= 128) {
        tlen = 127;
    }

    memcpy(table[id].text, text, (size_t)tlen);
    table[id].text[tlen] = '\0';
    table[id].active = 1;
    return 0;
}

const char *override_core_get(const OverrideCoreEntry *table, int num_ids, int id)
{
    if (!table || id < 0 || id >= num_ids) {
        return NULL;
    }
    if (!table[id].active) {
        return NULL;
    }
    return table[id].text;
}

void override_core_clear(OverrideCoreEntry *table, int num_ids, int id)
{
    if (table && id >= 0 && id < num_ids) {
        table[id].active = 0;
        table[id].text[0] = '\0';
    }
}
