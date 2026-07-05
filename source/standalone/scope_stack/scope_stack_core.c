/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: scope_stack_core.c
 * Subsystem: Nested Call-Frame Stack Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Maintains nested scope contexts for subroutine variables.
 *
 * 2. WHAT TO EXPECT:
 *    Tracks active local frames and restores values.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Max depth limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Local variable tracking rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If scope stack overflows, check call recursion depth.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE VARIABLE SCOPE STACK ENGINE
 * File: scope_stack_core.c
 * ===================================================================== */

#include "scope_stack_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scope_stack_core_init(ScopeStack *ss)
{
    ss->depth = 0;
    ss->capacity = SCOPE_INITIAL_DEPTH;
    ss->levels = (ScopeSnapshot *)calloc((size_t)ss->capacity, sizeof(ScopeSnapshot));
    if (ss->levels == NULL) {
        fprintf(stderr, "FATAL: Cannot allocate scope stack (%d levels)\n", ss->capacity);
        ss->capacity = 0;
    }
}

void scope_stack_core_free(ScopeStack *ss)
{
    if (ss->levels != NULL) {
        int i;
        for (i = 0; i < ss->depth; i++) {
            if (ss->levels[i].named_vars != NULL) {
                free(ss->levels[i].named_vars);
                ss->levels[i].named_vars = NULL;
            }
        }
        free(ss->levels);
        ss->levels = NULL;
    }
    ss->depth = 0;
    ss->capacity = 0;
}

int scope_stack_core_grow(ScopeStack *ss)
{
    int new_cap = ss->capacity * 2;
    if (new_cap > SCOPE_MAX_DEPTH)
        new_cap = SCOPE_MAX_DEPTH;
    if (new_cap <= ss->capacity)
        return -1; // already at max

    ScopeSnapshot *new_levels = (ScopeSnapshot *)realloc(
        ss->levels, (size_t)new_cap * sizeof(ScopeSnapshot));
    if (new_levels == NULL)
        return -1;

    // Zero the new entries
    memset(&new_levels[ss->capacity], 0,
           (size_t)(new_cap - ss->capacity) * sizeof(ScopeSnapshot));

    ss->levels = new_levels;
    ss->capacity = new_cap;
    return 0;
}

int scope_stack_core_push(ScopeStack *ss, 
                          const BValue *variables, 
                          const BValue *string_vars,
                          const void *named_vars_src, 
                          int named_count,
                          size_t named_var_size,
                          int mode, int sub_index, int return_idx)
{
    ScopeSnapshot *snap;
    int i;
    size_t nv_size;

    if (ss->levels == NULL) return -1;

    // Grow if needed
    if (ss->depth >= ss->capacity) {
        if (scope_stack_core_grow(ss) != 0) {
            printf("HOW? Scope stack overflow (depth %d)\n", ss->depth);
            return -1;
        }
    }

    snap = &ss->levels[ss->depth];
    memset(snap, 0, sizeof(ScopeSnapshot));

    // Save single-letter vars A-Z
    for (i = 0; i < MAX_VARIABLES; i++)
        snap->saved_vars[i] = variables[i];

    // Save string vars A$-Z$
    for (i = 0; i < MAX_STRING_VARS; i++)
        snap->saved_strvars[i] = string_vars[i];

    // Save named variables (dynamically allocated copy)
    snap->named_count = named_count;
    if (named_count > 0 && named_vars_src) {
        nv_size = (size_t)named_count * named_var_size;
        snap->named_vars = malloc(nv_size);
        if (snap->named_vars == NULL) {
            printf("HOW? Cannot allocate scope snapshot (%d named vars)\n", named_count);
            return -1;
        }
        memcpy(snap->named_vars, named_vars_src, nv_size);
    } else {
        snap->named_vars = NULL;
    }

    // Store metadata
    snap->scope_mode = mode;
    snap->sub_index = sub_index;
    snap->return_index = return_idx;
    snap->local_count = 0;
    snap->shared_count = 0;

    ss->depth++;
    return 0;
}

int scope_stack_core_pop(ScopeStack *ss,
                         BValue *variables,
                         BValue *string_vars,
                         void *named_vars_dest,
                         int *named_count_ptr,
                         size_t named_var_size,
                         ScopeStackGetNamedFn get_named_cb,
                         ScopeStackSetNamedFn set_named_cb,
                         void *cb_user_data)
{
    ScopeSnapshot *snap;
    int i;

    if (ss->levels == NULL || ss->depth <= 0)
        return -1;

    ss->depth--;
    snap = &ss->levels[ss->depth];

    // Step 1: Restore LOCAL-saved values.
    for (i = snap->local_count - 1; i >= 0; i--) {
        LocalSave *ls = &snap->locals[i];
        if (ls->is_single_letter) {
            int vi = ls->var_letter - 'A';
            if (ls->is_string_var) {
                string_vars[vi] = ls->value;
            } else {
                variables[vi] = ls->value;
            }
        } else {
            if (set_named_cb) {
                set_named_cb(cb_user_data, ls->name, ls->name_len, ls->value);
            }
        }
    }

    // Step 2: Restore the full snapshot.
    if (snap->shared_count == 0) {
        for (i = 0; i < MAX_VARIABLES; i++)
            variables[i] = snap->saved_vars[i];
        for (i = 0; i < MAX_STRING_VARS; i++)
            string_vars[i] = snap->saved_strvars[i];
    } else {
        // Save current values of SHARED single-letter vars
        BValue shared_sv[26];
        BValue shared_ssv[26];
        int is_shared_v[26];
        int is_shared_sv[26];
        int j;
        memset(is_shared_v, 0, sizeof(is_shared_v));
        memset(is_shared_sv, 0, sizeof(is_shared_sv));
        for (j = 0; j < snap->shared_count; j++) {
            char c = snap->shared_names[j][0];
            int slen = (int)strlen(snap->shared_names[j]);
            if (slen == 1 && c >= 'A' && c <= 'Z') {
                int vi = c - 'A';
                shared_sv[vi] = variables[vi];
                is_shared_v[vi] = 1;
            } else if (slen == 2 && c >= 'A' && c <= 'Z' && snap->shared_names[j][1] == '$') {
                int vi = c - 'A';
                shared_ssv[vi] = string_vars[vi];
                is_shared_sv[vi] = 1;
            }
        }
        // Bulk restore
        for (i = 0; i < MAX_VARIABLES; i++)
            variables[i] = snap->saved_vars[i];
        for (i = 0; i < MAX_STRING_VARS; i++)
            string_vars[i] = snap->saved_strvars[i];
        // Re-apply shared var current values
        for (i = 0; i < 26; i++) {
            if (is_shared_v[i])
                variables[i] = shared_sv[i];
            if (is_shared_sv[i])
                string_vars[i] = shared_ssv[i];
        }
    }

    // Restore named variables
    if (snap->named_vars != NULL) {
        if (snap->shared_count == 0) {
            if (named_vars_dest) {
                memcpy(named_vars_dest, snap->named_vars, (size_t)snap->named_count * named_var_size);
            }
            if (named_count_ptr) {
                *named_count_ptr = snap->named_count;
            }
        } else {
            // SHARED vars present: restore all except shared.
            int j;
            BValue shared_vals[SCOPE_MAX_LOCALS];
            char shared_nms[SCOPE_MAX_LOCALS][MAX_VAR_NAME_LEN + 1];
            int shared_lens[SCOPE_MAX_LOCALS];
            int sc = snap->shared_count;

            for (j = 0; j < sc && j < SCOPE_MAX_LOCALS; j++) {
                int slen = (int)strlen(snap->shared_names[j]);
                if (get_named_cb) {
                    shared_vals[j] = get_named_cb(cb_user_data, snap->shared_names[j], slen);
                } else {
                    memset(&shared_vals[j], 0, sizeof(BValue));
                }
                memcpy(shared_nms[j], snap->shared_names[j], (size_t)(slen + 1));
                shared_lens[j] = slen;
            }

            // Bulk restore
            if (named_vars_dest) {
                memcpy(named_vars_dest, snap->named_vars, (size_t)snap->named_count * named_var_size);
            }
            if (named_count_ptr) {
                *named_count_ptr = snap->named_count;
            }

            // Re-apply shared variable values
            for (j = 0; j < sc && j < SCOPE_MAX_LOCALS; j++) {
                if (set_named_cb) {
                    set_named_cb(cb_user_data, shared_nms[j], shared_lens[j], shared_vals[j]);
                }
            }
        }

        free(snap->named_vars);
        snap->named_vars = NULL;
    } else {
        if (named_count_ptr) {
            *named_count_ptr = snap->named_count;
        }
    }

    return 0;
}

int scope_stack_core_add_local(ScopeStack *ss,
                               BValue *variables,
                               BValue *string_vars,
                               ScopeStackGetNamedFn get_named_cb,
                               ScopeStackSetNamedFn set_named_cb,
                               void *cb_user_data,
                               const char *name, int name_len,
                               char var_letter, int is_string)
{
    ScopeSnapshot *snap;
    LocalSave *ls;

    if (ss->levels == NULL || ss->depth <= 0)
        return -1; // no active scope

    snap = &ss->levels[ss->depth - 1];

    if (snap->local_count >= SCOPE_MAX_LOCALS)
        return -1; // locals list full

    ls = &snap->locals[snap->local_count];
    memset(ls, 0, sizeof(LocalSave));

    if (var_letter >= 'A' && var_letter <= 'Z') {
        // Single-letter variable
        ls->is_single_letter = 1;
        ls->var_letter = var_letter;
        ls->is_string_var = is_string;
        if (is_string) {
            ls->value = string_vars[var_letter - 'A'];
            memset(&string_vars[var_letter - 'A'], 0, sizeof(BValue));
        } else {
            ls->value = variables[var_letter - 'A'];
            memset(&variables[var_letter - 'A'], 0, sizeof(BValue));
        }
    } else if (name != NULL && name_len > 0) {
        // Named variable
        int clen = name_len;
        if (clen > MAX_VAR_NAME_LEN)
            clen = MAX_VAR_NAME_LEN;
        memcpy(ls->name, name, (size_t)clen);
        ls->name[clen] = '\0';
        ls->name_len = clen;
        ls->is_single_letter = 0;
        if (get_named_cb) {
            ls->value = get_named_cb(cb_user_data, name, name_len);
        }
        if (set_named_cb) {
            BValue zero_val;
            memset(&zero_val, 0, sizeof(BValue));
            set_named_cb(cb_user_data, name, name_len, zero_val);
        }
    } else {
        return -1;
    }

    snap->local_count++;
    return 0;
}

int scope_stack_core_add_shared(ScopeStack *ss, const char *name, int name_len)
{
    ScopeSnapshot *snap;
    int clen;

    if (ss->levels == NULL || ss->depth <= 0)
        return -1;

    snap = &ss->levels[ss->depth - 1];

    if (snap->shared_count >= SCOPE_MAX_LOCALS)
        return -1;

    clen = name_len;
    if (clen > MAX_VAR_NAME_LEN)
        clen = MAX_VAR_NAME_LEN;
    memcpy(snap->shared_names[snap->shared_count], name, (size_t)clen);
    snap->shared_names[snap->shared_count][clen] = '\0';
    snap->shared_count++;

    return 0;
}
