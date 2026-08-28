// FILENAME: variables.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for variables within BASIC++.
//
// ---- Includes ----

#include "runtime/variables_internal.h"

//
// ---- Lifecycle & Clearing ----

VariableContext *var_init(MemoryContext *mem, StringContext *str) {
    if (!mem || !str) return NULL;
    VariableContext *ctx = (VariableContext *)calloc(1, sizeof(VariableContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->str = str;
    ctx->is_explicit = false;
    ctx->active_scope[0] = '\0';
    ctx->active_namespace[0] = '\0';
    ctx->shared_count = 0;
    ctx->common_count = 0;
    memset(ctx->buckets, 0, sizeof(ctx->buckets));
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
    free(ctx);
}

void var_clear_all(VariableContext *ctx) {
    if (!ctx) return;

    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            VarEntry *next = entry->next;
            free(entry->name);
            if ((entry->value.type == VAL_STRING || entry->value.type == VAL_ARRAY_REF) && entry->value.as.string) {
                str_release(ctx->str, entry->value.as.string);
            } else if (entry->value.type == VAL_MAP && entry->value.as.map) {
                map_release(ctx->str, entry->value.as.map);
            }
            free(entry);
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
        free(curr);
        curr = next;
    }
    ctx->scope_defs = NULL;

    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
    memset(ctx->mru_cache, 0, sizeof(ctx->mru_cache));
    ctx->mru_head = 0;
}
