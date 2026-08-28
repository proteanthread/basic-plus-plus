// FILENAME: scope.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libcore (memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (scope.h, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for scope within BASIC++.
//
// ---- Includes ----

#include "scope/scope.h"
#include "types/types.h"
#include "vm/vm.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

// Global Scope State storage for VM fallback or direct tracking
static BppScopeState g_scope_state;

static BppScopeState *get_scope(VMContext *vm) {
    (void)vm;
    return &g_scope_state;
}

void scope_init(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (s) {
        runtime_memset(s, 0, sizeof(BppScopeState));
    }
}

void scope_clear(VMContext *vm) {
    scope_init(vm);
}

// Keyword Disable / Enable
bool scope_keyword_disable(VMContext *vm, const char *keyword) {
    if (!keyword) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    if (scope_is_keyword_disabled(vm, keyword)) return true;
    if (s->disabled_count >= MAX_DISABLED_KEYWORDS) return false;

    runtime_strncpy(s->disabled_keywords[s->disabled_count], keyword, sizeof(s->disabled_keywords[0]) - 1);
    s->disabled_keywords[s->disabled_count][sizeof(s->disabled_keywords[0]) - 1] = '\0';
    s->disabled_count++;
    return true;
}

bool scope_keyword_enable(VMContext *vm, const char *keyword) {
    if (!keyword) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->disabled_count; ++i) {
        if (runtime_strcasecmp(s->disabled_keywords[i], keyword) == 0) {
            for (int j = i; j < s->disabled_count - 1; ++j) {
                runtime_memcpy(s->disabled_keywords[j], s->disabled_keywords[j + 1], sizeof(s->disabled_keywords[0]));
            }
            runtime_memset(s->disabled_keywords[s->disabled_count - 1], 0, sizeof(s->disabled_keywords[0]));
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
        if (runtime_strcasecmp(s->disabled_keywords[i], keyword) == 0) {
            return true;
        }
    }
    return false;
}

// Execution Hooks
bool scope_register_hook(VMContext *vm, const char *cmd, BppHookType type, const char *target_label, BppLineNumber line) {
    if (!cmd) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->hook_count >= MAX_HOOKS) return false;

    BppScopeHook *h = &s->hooks[s->hook_count];
    runtime_memset(h, 0, sizeof(BppScopeHook));

    runtime_strncpy(h->target_cmd, cmd, sizeof(h->target_cmd) - 1);
    h->type = type;
    if (target_label) {
        runtime_strncpy(h->target_label, target_label, sizeof(h->target_label) - 1);
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
        if (s->hooks[i].type == type && runtime_strcasecmp(s->hooks[i].target_cmd, cmd) == 0) {
            return &s->hooks[i];
        }
    }
    return NULL;
}

void scope_clear_hooks(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (s) {
        s->hook_count = 0;
        runtime_memset(s->hooks, 0, sizeof(s->hooks));
    }
}

// Visibility & Protection
bool scope_set_symbol_private(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->private_count >= MAX_PRIVATE_SYMBOLS) return false;

    if (scope_is_symbol_private(vm, symbol)) return true;

    runtime_strncpy(s->private_symbols[s->private_count], symbol, sizeof(s->private_symbols[0]) - 1);
    s->private_symbols[s->private_count][sizeof(s->private_symbols[0]) - 1] = '\0';
    s->private_count++;
    return true;
}

bool scope_is_symbol_private(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->private_count; ++i) {
        if (runtime_strcasecmp(s->private_symbols[i], symbol) == 0) {
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

    runtime_strncpy(s->protected_symbols[s->protected_count], symbol, sizeof(s->protected_symbols[0]) - 1);
    s->protected_symbols[s->protected_count][sizeof(s->protected_symbols[0]) - 1] = '\0';
    s->protected_count++;
    return true;
}

bool scope_is_symbol_protected(VMContext *vm, const char *symbol) {
    if (!symbol) return false;
    BppScopeState *s = get_scope(vm);
    if (!s) return false;

    for (int i = 0; i < s->protected_count; ++i) {
        if (runtime_strcasecmp(s->protected_symbols[i], symbol) == 0) {
            return true;
        }
    }
    return false;
}

// Namespace Scoping
bool scope_namespace_enter(VMContext *vm, const char *ns_name) {
    if (!ns_name) return false;
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth >= MAX_NAMESPACE_DEPTH) return false;

    runtime_strncpy(s->namespace_stack[s->namespace_depth], ns_name, sizeof(s->namespace_stack[0]) - 1);
    s->namespace_stack[s->namespace_depth][sizeof(s->namespace_stack[0]) - 1] = '\0';
    s->namespace_depth++;
    return true;
}

bool scope_namespace_exit(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth <= 0) return false;

    s->namespace_depth--;
    runtime_memset(s->namespace_stack[s->namespace_depth], 0, sizeof(s->namespace_stack[0]));
    return true;
}

const char *scope_get_current_namespace(VMContext *vm) {
    BppScopeState *s = get_scope(vm);
    if (!s || s->namespace_depth <= 0) return "";
    return s->namespace_stack[s->namespace_depth - 1];
}

// Block Scoping
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
