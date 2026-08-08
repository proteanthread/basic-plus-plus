/**
 * @file scope.c
 * @brief Subsystem implementation for SCOPE rules, keyword isolation, namespace protection, and block scoping in BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements core scoping rules, symbol privacy, keyword disabling, namespace isolation, execution hooks, and block scoping state management (BppScopeState).
 *
 * 2. WHY IT EXISTS:
 * Enforces runtime isolation, security policies, and modular scoping rules across dialects and user modules.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maintains a stack of active BppScopeState frames zero-initialized by default; performs length-checked bounded string comparisons and keyword rule lookups without heap fragmentation.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'scope'. Includes "scope/scope.h", "types/types.h", "vm/vm.h",
 * <string.h>, <ctype.h>, <stdlib.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add additional scope policy flags to BppScopeState struct in scope/scope.h.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Bounded string comparison invariants and zero-initialization rules.
 *
 * 8. WHAT TO EXPECT:
 * Manages scope stack frames and returns BppError code on privilege or isolation violation.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string copy lengths and check zero-initialization of scope frame structures.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Cross-platform case-insensitive string comparison wrapper (strncasecmp / _strnicmp).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/vm.c
 * Prerequisite Header Files:
 * - engine/include/scope/scope.h
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 */

#include "scope/scope.h"
#include "types/types.h"
#include "vm/vm.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/* Global Scope State storage for VM fallback or direct tracking */
static BppScopeState g_scope_state;

static BppScopeState *get_scope(VMContext *vm) {
    (void)vm;
    return &g_scope_state;
}

void scope_init(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (s) {
        memset(s, 0, sizeof(BppScopeState));
    }
}

void scope_clear(VMContext *vm) {
    scope_init(vm);
}

/* Keyword Disable / Enable */
bool scope_keyword_disable(VMContext *vm, const char *keyword) {
    if (!keyword) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    if (scope_is_keyword_disabled(vm, keyword)) return true;
    if (s->disabled_count >= MAX_DISABLED_KEYWORDS) return false;

    strncpy(s->disabled_keywords[s->disabled_count], keyword, sizeof(s->disabled_keywords[0]) - 1);
    s->disabled_keywords[s->disabled_count][sizeof(s->disabled_keywords[0]) - 1] = '\0';
    s->disabled_count++;
    return true;
}

bool scope_keyword_enable(VMContext *vm, const char *keyword) {
    if (!keyword) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->disabled_count; ++i) {
        if (strcasecmp(s->disabled_keywords[i], keyword) == 0) {
            for (int j = i; j < s->disabled_count - 1; ++j) {
                memcpy(s->disabled_keywords[j], s->disabled_keywords[j + 1], sizeof(s->disabled_keywords[0]));
            }
            memset(s->disabled_keywords[s->disabled_count - 1], 0, sizeof(s->disabled_keywords[0]));
            s->disabled_count--;
            return true;
        }
    }
    return false;
}

bool scope_is_keyword_disabled(VMContext *vm, const char *keyword) {
    if (!keyword) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->disabled_count; ++i) {
        if (strcasecmp(s->disabled_keywords[i], keyword) == 0) {
            return true;
        }
    }
    return false;
}

/* Execution Hooks */
bool scope_register_hook(VMContext *vm, const char *cmd, BppHookType type, const char *target_label, BppLineNumber line) {
    if (!cmd) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->hook_count >= MAX_HOOKS) return false;

    BppScopeHook *h = &s->hooks[s->hook_count];
    memset(h, 0, sizeof(BppScopeHook));

    strncpy(h->target_cmd, cmd, sizeof(h->target_cmd) - 1);
    h->type = type;
    if (target_label) {
        strncpy(h->target_label, target_label, sizeof(h->target_label) - 1);
    }
    h->target_line = line;
    s->hook_count++;
    return true;
}

const BppScopeHook *scope_lookup_hook(VMContext *vm, const char *cmd, BppHookType type) {
    if (!cmd) return NULL;
    BppScopeState *s = get_scope(vm);
    if (!s) return NULL;

    for (int i = 0; i < s->hook_count; ++i) {
        if (s->hooks[i].type == type && strcasecmp(s->hooks[i].target_cmd, cmd) == 0) {
            return &s->hooks[i];
        }
    }
    return NULL;
}

void scope_clear_hooks(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (s) {
        s->hook_count = 0;
        memset(s->hooks, 0, sizeof(s->hooks));
    }
}

/* Visibility & Protection */
bool scope_set_symbol_private(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->private_count >= MAX_PRIVATE_SYMBOLS) return false;

    if (scope_is_symbol_private(vm, symbol)) return true;

    strncpy(s->private_symbols[s->private_count], symbol, sizeof(s->private_symbols[0]) - 1);
    s->private_symbols[s->private_count][sizeof(s->private_symbols[0]) - 1] = '\0';
    s->private_count++;
    return true;
}

bool scope_is_symbol_private(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->private_count; ++i) {
        if (strcasecmp(s->private_symbols[i], symbol) == 0) {
            return true;
        }
    }
    return false;
}

bool scope_protect_symbol(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->protected_count >= MAX_PROTECTED_SYMBOLS) return false;

    if (scope_is_symbol_protected(vm, symbol)) return true;

    strncpy(s->protected_symbols[s->protected_count], symbol, sizeof(s->protected_symbols[0]) - 1);
    s->protected_symbols[s->protected_count][sizeof(s->protected_symbols[0]) - 1] = '\0';
    s->protected_count++;
    return true;
}

bool scope_is_symbol_protected(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->protected_count; ++i) {
        if (strcasecmp(s->protected_symbols[i], symbol) == 0) {
            return true;
        }
    }
    return false;
}

/* Namespace Scoping */
bool scope_namespace_enter(VMContext *vm, const char *ns_name) {
    if (!ns_name) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth >= MAX_NAMESPACE_DEPTH) return false;

    strncpy(s->namespace_stack[s->namespace_depth], ns_name, sizeof(s->namespace_stack[0]) - 1);
    s->namespace_stack[s->namespace_depth][sizeof(s->namespace_stack[0]) - 1] = '\0';
    s->namespace_depth++;
    return true;
}

bool scope_namespace_exit(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth <= 0) return false;

    s->namespace_depth--;
    memset(s->namespace_stack[s->namespace_depth], 0, sizeof(s->namespace_stack[0]));
    return true;
}

const char *scope_get_current_namespace(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth <= 0) return "";
    return s->namespace_stack[s->namespace_depth - 1];
}

/* Block Scoping */
bool scope_block_push(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->block_depth >= MAX_SCOPE_BLOCKS) return false;
    s->block_depth++;
    return true;
}

bool scope_block_pop(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->block_depth <= 0) return false;
    s->block_depth--;
    return true;
}

int scope_get_block_depth(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    return s ? s->block_depth : 0;
}
