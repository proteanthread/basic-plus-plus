/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: scope_core.c
 * Subsystem: Keyword Access Scope Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Enforces keyword execution access rules and hooks.
 *
 * 2. WHAT TO EXPECT:
 *    Blocks disabled keywords and runs GOSUB hooks.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Access presets maps.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Access control lookup routines.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If command fails with access error, check rules lists.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE KEYWORD ACCESS CONTROL SCOPE REGISTRY CORE
 * File: scope_core.c
 * ===================================================================== */

#include "scope_core.h"

static int is_protected(KeywordId kw)
{
    return (kw == KW_SCOPE || kw == KW_ALIAS ||
            kw == KW_KEYWORD || kw == KW_REM ||
            kw == KW_SECURITY || kw == KW_OVERRIDE);
}

void scope_core_init(ScopeEntryCore *table, int count, int *last_hook)
{
    if (table) {
        int i;
        for (i = 0; i < count; i++) {
            table[i].disabled = 0;
            table[i].before_line = -1;
            table[i].after_line = -1;
            table[i].override_line = -1;
        }
    }
    if (last_hook) {
        *last_hook = count;
    }
}

int scope_core_is_disabled(const ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return 0;
    return table[kw].disabled;
}

int scope_core_disable(ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return -1;
    if (is_protected(kw)) return -2;
    table[kw].disabled = 1;
    return 0;
}

int scope_core_enable(ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return -1;
    table[kw].disabled = 0;
    return 0;
}

int scope_core_set_before(ScopeEntryCore *table, int count, KeywordId kw, int line)
{
    if (!table || kw < 0 || kw >= count) return -1;
    if (is_protected(kw)) return -2;
    table[kw].before_line = line;
    return 0;
}

int scope_core_set_after(ScopeEntryCore *table, int count, KeywordId kw, int line)
{
    if (!table || kw < 0 || kw >= count) return -1;
    if (is_protected(kw)) return -2;
    table[kw].after_line = line;
    return 0;
}

int scope_core_set_override(ScopeEntryCore *table, int count, KeywordId kw, int line)
{
    if (!table || kw < 0 || kw >= count) return -1;
    if (is_protected(kw)) return -2;
    table[kw].override_line = line;
    return 0;
}

int scope_core_get_before(const ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return -1;
    return table[kw].before_line;
}

int scope_core_get_after(const ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return -1;
    return table[kw].after_line;
}

int scope_core_get_override(const ScopeEntryCore *table, int count, KeywordId kw)
{
    if (!table || kw < 0 || kw >= count) return -1;
    return table[kw].override_line;
}

void scope_core_clear_hooks(ScopeEntryCore *table, int count, KeywordId kw)
{
    if (table && kw >= 0 && kw < count) {
        table[kw].before_line = -1;
        table[kw].after_line = -1;
        table[kw].override_line = -1;
    }
}

void scope_core_restore(ScopeEntryCore *table, int count, KeywordId kw)
{
    if (table && kw >= 0 && kw < count) {
        table[kw].disabled = 0;
        scope_core_clear_hooks(table, count, kw);
    }
}

int scope_core_has_any_rules(const ScopeEntryCore *table, int count)
{
    if (table) {
        int i;
        for (i = 0; i < count; i++) {
            if (table[i].disabled ||
                table[i].before_line >= 0 ||
                table[i].after_line >= 0 ||
                table[i].override_line >= 0) {
                return 1;
            }
        }
    }
    return 0;
}
