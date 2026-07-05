/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: scope_core.h
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
 * File: scope_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_SCOPE_CORE_H
#define BASICPP_STANDALONE_SCOPE_CORE_H

#include "../../lexer.h"

typedef struct ScopeEntryCore {
    int disabled;
    int before_line;
    int after_line;
    int override_line;
} ScopeEntryCore;

void scope_core_init(ScopeEntryCore *table, int count, int *last_hook);
int scope_core_is_disabled(const ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_disable(ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_enable(ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_set_before(ScopeEntryCore *table, int count, KeywordId kw, int line);
int scope_core_set_after(ScopeEntryCore *table, int count, KeywordId kw, int line);
int scope_core_set_override(ScopeEntryCore *table, int count, KeywordId kw, int line);
int scope_core_get_before(const ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_get_after(const ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_get_override(const ScopeEntryCore *table, int count, KeywordId kw);
void scope_core_clear_hooks(ScopeEntryCore *table, int count, KeywordId kw);
void scope_core_restore(ScopeEntryCore *table, int count, KeywordId kw);
int scope_core_has_any_rules(const ScopeEntryCore *table, int count);

#endif // BASICPP_STANDALONE_SCOPE_CORE_H
