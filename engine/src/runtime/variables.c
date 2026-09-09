// FILENAME: variables.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for variables within BASIC++.
//
// ---- Includes ----

#include "runtime/variables_internal.h"
#include "runtime/string/memops.h"
#include "runtime/memory/alloc.h"

//
// ---- Lifecycle & Clearing ----

VariableContext *var_init(MemoryContext *mem, StringContext *str) {
    if (!mem || !str) return NULL;
    VariableContext *ctx = (VariableContext *)runtime_calloc(1, sizeof(VariableContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->str = str;
    ctx->is_explicit = false;
    ctx->active_scope[0] = '\0';
    ctx->active_namespace[0] = '\0';
    ctx->shared_count = 0;
    ctx->common_count = 0;
    runtime_memset(ctx->buckets, 0, sizeof(ctx->buckets));
    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
    ctx->scope_defs = NULL;
    ctx->case_sensitive = false;
    return ctx;
}

void var_shutdown(VariableContext *ctx) {
    if (!ctx) return;
    var_clear_all(ctx);
    DynamicVarEntry *dvar = ctx->dynamic_vars;
    while (dvar) {
        DynamicVarEntry *next = dvar->next;
        runtime_free(dvar);
        dvar = next;
    }
    ctx->dynamic_vars = NULL;
    runtime_free(ctx);
}

void var_clear_all(VariableContext *ctx) {
    if (!ctx) return;

    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            VarEntry *next = entry->next;
            runtime_free(entry->name);
            if ((entry->value.type == VAL_STRING || entry->value.type == VAL_ARRAY_REF) && entry->value.as.string) {
                str_release(ctx->str, entry->value.as.string);
            } else if (entry->value.type == VAL_MAP && entry->value.as.map) {
                map_release(ctx->str, entry->value.as.map);
            }
            runtime_free(entry);
            entry = next;
        }
        ctx->buckets[i] = NULL;
    }
    ctx->is_explicit = false;
    ctx->shared_count = 0;
    ctx->common_count = 0;

    ScopeDefMapping *curr = ctx->scope_defs;
    while (curr) {
        ScopeDefMapping *next = curr->next;
        runtime_free(curr);
        curr = next;
    }
    ctx->scope_defs = NULL;

    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
    runtime_memset(ctx->fast_scalars_valid, 0, sizeof(ctx->fast_scalars_valid));
    runtime_memset(ctx->fast_scalars, 0, sizeof(ctx->fast_scalars));
    runtime_memset(ctx->direct_cache, 0, sizeof(ctx->direct_cache));
}
