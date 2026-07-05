/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: scope_stack.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Self-describing HELP interactive system, OPTION overrides, and security scope permissions.
 *
 * 2. WHAT TO EXPECT:
 *    Help prints keyword usage. Overrides rewrite token attributes at parse time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Command summaries, help topics, scope presets mappings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Help databases lookup logic, keyword gating routines.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If help command fails, ensure help databases are sorted. Check override loop guards.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - scope_stack.c
 // ---
 //
 // Dynamic scope stack implementation.
 //
 // Provides variable isolation for SUB/FUNCTION calls with
 // two modes: full snapshot (ECMA-116) and fresh scope (QBasic).
 //
 // Memory is dynamically allocated and grows on demand.
 // Named variable snapshots are malloc'd per level.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "scope_stack.h"
#include "runtime.h"
#include "../console.h"

 // scope_stack_init - Allocate initial scope stack capacity.
void scope_stack_init(ScopeStack *ss)
{
    ss->depth = 0;
    ss->capacity = SCOPE_INITIAL_DEPTH;
    ss->levels = (ScopeSnapshot *)calloc(
        (size_t)ss->capacity, sizeof(ScopeSnapshot));
    if (ss->levels == NULL) {
        // Fatal: cannot allocate scope stack
        fprintf(stderr,
            "FATAL: Cannot allocate scope stack (%d levels)\n",
            ss->capacity);
        ss->capacity = 0;
    }
}

 // scope_stack_free - Release all scope stack memory.
 //
 // Frees any dynamically allocated named var snapshots
 // still on the stack, then frees the stack itself.
void scope_stack_free(ScopeStack *ss)
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

 // scope_stack_grow - Double the stack capacity.
 //
 // Returns 0 on success, -1 on failure (at max or malloc fail).
static int scope_stack_grow(ScopeStack *ss)
{
    int new_cap;
    ScopeSnapshot *new_levels;

    new_cap = ss->capacity * 2;
    if (new_cap > SCOPE_MAX_DEPTH)
        new_cap = SCOPE_MAX_DEPTH;
    if (new_cap <= ss->capacity)
        return -1; // already at max

    new_levels = (ScopeSnapshot *)realloc(
        ss->levels,
        (size_t)new_cap * sizeof(ScopeSnapshot));
    if (new_levels == NULL)
        return -1;

    // Zero the new entries
    memset(&new_levels[ss->capacity], 0,
        (size_t)(new_cap - ss->capacity) *
        sizeof(ScopeSnapshot));

    ss->levels = new_levels;
    ss->capacity = new_cap;
    return 0;
}

 // scope_stack_push - Save current variable state and push scope.
 //
 // Saves A-Z, A$-Z$, and all named variables.
 // If mode == SCOPE_FRESH, zeros all variables after saving.
int scope_stack_push(ScopeStack *ss, struct RuntimeState *rt,
                     int mode, int sub_index, int return_idx)
{
    ScopeSnapshot *snap;
    int i;
    size_t nv_size;

    if (ss->levels == NULL) return -1;

    // Grow if needed
    if (ss->depth >= ss->capacity) {
        if (scope_stack_grow(ss) != 0) {
            printf("HOW? Scope stack overflow "
                   "(depth %d)\n", ss->depth);
            return -1;
        }
    }

    snap = &ss->levels[ss->depth];
    memset(snap, 0, sizeof(ScopeSnapshot));

    // Save single-letter vars A-Z
    for (i = 0; i < MAX_VARIABLES; i++)
        snap->saved_vars[i] = rt->variables[i];

    // Save string vars A$-Z$
    for (i = 0; i < MAX_STRING_VARS; i++)
        snap->saved_strvars[i] = rt->string_vars[i];

    // Save named variables (dynamically allocated copy)
    snap->named_count = rt->named_count;
    if (rt->named_count > 0) {
        nv_size = (size_t)rt->named_count *
                  sizeof(NamedVariable);
        snap->named_vars = malloc(nv_size);
        if (snap->named_vars == NULL) {
            printf("HOW? Cannot allocate scope snapshot "
                   "(%d named vars)\n", rt->named_count);
            return -1;
        }
        memcpy(snap->named_vars, rt->named_vars, nv_size);
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

    // If SCOPE_FRESH (QBasic style): zero all vars for clean scope
    if (mode == SCOPE_FRESH) {
        for (i = 0; i < MAX_VARIABLES; i++)
            rt->variables[i] = bval_int(0);
        for (i = 0; i < MAX_STRING_VARS; i++)
            rt->string_vars[i] = bval_int(0);
        // Named vars: reset count to 0 (fresh namespace)
        rt->named_count = 0;
    }

    return 0;
}

 // is_shared - Check if a variable name is in the SHARED list.
#if 0
static int is_shared_named(ScopeSnapshot *snap,
                           const char *name, int name_len)
{
    int i;
    for (i = 0; i < snap->shared_count; i++) {
        if (strncmp(snap->shared_names[i], name,
                    (size_t)name_len) == 0 &&
            snap->shared_names[i][name_len] == '\0') {
            return 1;
        }
    }
    return 0;
}
#endif

 // scope_stack_pop - Restore saved variable state and pop scope.
 //
 // Restores LOCAL-saved values first, then the full snapshot.
 // SHARED variables are NOT restored (changes propagate back).
int scope_stack_pop(ScopeStack *ss, struct RuntimeState *rt)
{
    ScopeSnapshot *snap;
    int i;

    if (ss->levels == NULL || ss->depth <= 0)
        return -1;

    ss->depth--;
    snap = &ss->levels[ss->depth];

     // Step 0: Save STATIC variables back to SubDef.
     // Must happen before any restore operations.
     // Note: depth already decremented, so save_static
     // won't find the snap. Temporarily bump depth.
    ss->depth++;
    scope_stack_save_static(ss, rt);
    ss->depth--;

     // Step 1: Restore LOCAL-saved values.
     // These are variables that were explicitly declared LOCal
     // inside the SUB body. They are restored before the full
     // snapshot so their original caller values are preserved.
    for (i = snap->local_count - 1; i >= 0; i--) {
        LocalSave *ls = &snap->locals[i];
        if (ls->is_single_letter) {
            int vi = ls->var_letter - 'A';
            if (ls->is_string_var) {
                rt->string_vars[vi] = ls->value;
            } else {
                rt->variables[vi] = ls->value;
            }
        } else {
            // Named variable: restore by name
            runtime_set_named_var_bval(rt,
                ls->name, ls->name_len, ls->value);
        }
    }

     // Step 2: Restore the full snapshot.
     // Skip SHARED variables -- their current values should
     // propagate back to the caller.
    // Restore A-Z (skip SHARED single-letter vars)
    if (snap->shared_count == 0) {
        for (i = 0; i < MAX_VARIABLES; i++)
            rt->variables[i] = snap->saved_vars[i];
        for (i = 0; i < MAX_STRING_VARS; i++)
            rt->string_vars[i] = snap->saved_strvars[i];
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
            int slen = (int)strlen(
                snap->shared_names[j]);
            if (slen == 1 && c >= 'A' && c <= 'Z') {
                int vi = c - 'A';
                shared_sv[vi] = rt->variables[vi];
                is_shared_v[vi] = 1;
            } else if (slen == 2 && c >= 'A' &&
                       c <= 'Z' &&
                       snap->shared_names[j][1] == '$') {
                int vi = c - 'A';
                shared_ssv[vi] = rt->string_vars[vi];
                is_shared_sv[vi] = 1;
            }
        }
        // Bulk restore
        for (i = 0; i < MAX_VARIABLES; i++)
            rt->variables[i] = snap->saved_vars[i];
        for (i = 0; i < MAX_STRING_VARS; i++)
            rt->string_vars[i] = snap->saved_strvars[i];
        // Re-apply shared var current values
        for (i = 0; i < 26; i++) {
            if (is_shared_v[i])
                rt->variables[i] = shared_sv[i];
            if (is_shared_sv[i])
                rt->string_vars[i] = shared_ssv[i];
        }
    }

    // Restore named variables
    if (snap->named_vars != NULL) {
        NamedVariable *saved =
            (NamedVariable *)snap->named_vars;

        if (snap->shared_count == 0) {
            // No SHARED vars: simple bulk restore
            memcpy(rt->named_vars, saved,
                (size_t)snap->named_count *
                sizeof(NamedVariable));
            rt->named_count = snap->named_count;
        } else {
             // SHARED vars present: restore all except shared.
             // For shared vars, keep the current (modified) value
             // by writing it into the restored namespace.
            int j;
            // First, collect current values of shared vars
            BValue shared_vals[SCOPE_MAX_LOCALS];
            char shared_nms[SCOPE_MAX_LOCALS]
                           [MAX_VAR_NAME_LEN + 1];
            int shared_lens[SCOPE_MAX_LOCALS];
            int sc = snap->shared_count;

            for (j = 0; j < sc && j < SCOPE_MAX_LOCALS; j++) {
                int slen = (int)strlen(
                    snap->shared_names[j]);
                shared_vals[j] =
                    runtime_get_named_var_bval(rt,
                        snap->shared_names[j], slen);
                memcpy(shared_nms[j],
                    snap->shared_names[j],
                    (size_t)(slen + 1));
                shared_lens[j] = slen;
            }

            // Bulk restore
            memcpy(rt->named_vars, saved,
                (size_t)snap->named_count *
                sizeof(NamedVariable));
            rt->named_count = snap->named_count;

            // Re-apply shared variable values
            for (j = 0; j < sc && j < SCOPE_MAX_LOCALS; j++) {
                runtime_set_named_var_bval(rt,
                    shared_nms[j], shared_lens[j],
                    shared_vals[j]);
            }
        }

        free(snap->named_vars);
        snap->named_vars = NULL;
    } else {
        rt->named_count = snap->named_count;
    }

    // Set return address
    rt->next_index = snap->return_index;
    rt->in_sub_index = -1;

    return 0;
}

 // scope_stack_depth - Return current nesting depth.
int scope_stack_depth(const ScopeStack *ss)
{
    return ss->depth;
}

 // scope_stack_add_local - Save a variable's value for LOCal restore.
 //
 // Called when LOCal is executed inside a SUB body.
 // Saves the current value, then the caller zeros the variable.
int scope_stack_add_local(ScopeStack *ss, struct RuntimeState *rt,
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
            ls->value = rt->string_vars[
                var_letter - 'A'];
            rt->string_vars[var_letter - 'A'] =
                bval_int(0);
        } else {
            ls->value = rt->variables[
                var_letter - 'A'];
            rt->variables[var_letter - 'A'] =
                bval_int(0);
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
        ls->value = runtime_get_named_var_bval(rt,
            name, name_len);
        runtime_set_named_var_bval(rt, name, name_len,
            bval_int(0));
    } else {
        return -1;
    }

    snap->local_count++;
    return 0;
}

 // scope_stack_add_shared - Mark a variable as SHARED.
 //
 // SHARED variables are not restored on scope pop.
int scope_stack_add_shared(ScopeStack *ss, const char *name,
                           int name_len)
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
    memcpy(snap->shared_names[snap->shared_count],
        name, (size_t)clen);
    snap->shared_names[snap->shared_count][clen] = '\0';
    snap->shared_count++;

    return 0;
}

 // scope_stack_add_static - Register a STATIC variable in current scope.
 //
 // Records the variable so it can be saved back to the SubDef
 // on scope exit. If the SubDef already has static data for
 // this variable, restores it now.
int scope_stack_add_static(ScopeStack *ss, struct RuntimeState *rt,
                           const char *name, int name_len,
                           char var_letter, int is_string)
{
    ScopeSnapshot *snap;
    int idx;

    if (ss->levels == NULL || ss->depth <= 0)
        return -1;

    snap = &ss->levels[ss->depth - 1];

    if (snap->static_count >= SCOPE_MAX_LOCALS)
        return -1;

    idx = snap->static_count;

    if (var_letter >= 'A' && var_letter <= 'Z') {
        // Single-letter variable
        snap->static_letters[idx] = var_letter;
        snap->static_names[idx][0] = '\0';
        snap->static_name_lens[idx] = 0;
        snap->static_is_string[idx] = is_string;

        // Restore from SubDef static storage if available
        if (snap->sub_index >= 0 &&
            snap->sub_index < rt->sub_count) {
            SubDef *sd = &rt->subs[snap->sub_index];
            if (sd->has_static_data) {
                int vi = var_letter - 'A';
                if (is_string && sd->static_strvars)
                    rt->string_vars[vi] =
                        sd->static_strvars[vi];
                else if (!is_string && sd->static_vars)
                    rt->variables[vi] =
                        sd->static_vars[vi];
            }
        }
    } else if (name != NULL && name_len > 0) {
        // Named variable
        int clen = name_len;
        if (clen > MAX_VAR_NAME_LEN)
            clen = MAX_VAR_NAME_LEN;
        memcpy(snap->static_names[idx], name,
            (size_t)clen);
        snap->static_names[idx][clen] = '\0';
        snap->static_name_lens[idx] = clen;
        snap->static_letters[idx] = 0;
        snap->static_is_string[idx] = 0;

        // Restore from SubDef static storage if available
        if (snap->sub_index >= 0 &&
            snap->sub_index < rt->sub_count) {
            SubDef *sd = &rt->subs[snap->sub_index];
            if (sd->has_static_data &&
                sd->static_named != NULL) {
                NamedVariable *snv =
                    (NamedVariable *)sd->static_named;
                int j;
                for (j = 0; j < sd->static_named_count;
                     j++) {
                    if ((int)strlen(snv[j].name) == clen &&
                        strncmp(snv[j].name, name,
                            (size_t)clen) == 0) {
                        runtime_set_named_var_bval(rt,
                            name, clen, snv[j].value);
                        break;
                    }
                }
            }
        }
    } else {
        return -1;
    }

    snap->static_count++;
    return 0;
}

 // scope_stack_save_static - Save static vars back to SubDef.
 //
 // Called from scope_stack_pop before restoring the caller's
 // variables. Saves current values of STATIC-marked variables
 // (or all vars if all_static) into the SubDef's static storage.
void scope_stack_save_static(ScopeStack *ss,
                             struct RuntimeState *rt)
{
    ScopeSnapshot *snap;
    SubDef *sd;
    int i;

    if (ss->levels == NULL || ss->depth <= 0)
        return;

    snap = &ss->levels[ss->depth - 1];

    // Nothing to save if no STATIC vars declared
    if (snap->static_count == 0 && !snap->all_static)
        return;

    if (snap->sub_index < 0 ||
        snap->sub_index >= rt->sub_count)
        return;

    sd = &rt->subs[snap->sub_index];

    // Allocate static storage on first use
    if (!sd->has_static_data) {
        sd->static_vars = (BValue *)calloc(
            MAX_VARIABLES, sizeof(BValue));
        sd->static_strvars = (BValue *)calloc(
            MAX_STRING_VARS, sizeof(BValue));
        sd->static_named = calloc(
            (size_t)rt->named_count,
            sizeof(NamedVariable));
        sd->static_named_count = rt->named_count;
        sd->has_static_data = 1;
    }

    if (snap->all_static) {
        // Save ALL current vars as static
        if (sd->static_vars) {
            for (i = 0; i < MAX_VARIABLES; i++)
                sd->static_vars[i] = rt->variables[i];
        }
        if (sd->static_strvars) {
            for (i = 0; i < MAX_STRING_VARS; i++)
                sd->static_strvars[i] =
                    rt->string_vars[i];
        }
        if (sd->static_named && rt->named_count > 0) {
            // Reallocate if named count grew
            if (rt->named_count >
                sd->static_named_count) {
                void *nn = realloc(sd->static_named,
                    (size_t)rt->named_count *
                    sizeof(NamedVariable));
                if (nn) {
                    sd->static_named = nn;
                    sd->static_named_count =
                        rt->named_count;
                }
            }
            memcpy(sd->static_named, rt->named_vars,
                (size_t)rt->named_count *
                sizeof(NamedVariable));
            sd->static_named_count = rt->named_count;
        }
    } else {
        // Save only individually-declared STATIC vars
        for (i = 0; i < snap->static_count; i++) {
            char vl = snap->static_letters[i];
            if (vl >= 'A' && vl <= 'Z') {
                int vi = vl - 'A';
                if (snap->static_is_string[i]) {
                    if (sd->static_strvars)
                        sd->static_strvars[vi] =
                            rt->string_vars[vi];
                } else {
                    if (sd->static_vars)
                        sd->static_vars[vi] =
                            rt->variables[vi];
                }
            } else if (snap->static_name_lens[i] > 0) {
                // Named var: find and save
                const char *sn = snap->static_names[i];
                int sl = snap->static_name_lens[i];
                BValue sv = runtime_get_named_var_bval(
                    rt, sn, sl);
                if (sd->static_named) {
                    NamedVariable *snv =
                        (NamedVariable *)
                        sd->static_named;
                    int j, found = 0;
                    for (j = 0;
                         j < sd->static_named_count;
                         j++) {
                        if ((int)strlen(snv[j].name) == sl &&
                            strncmp(snv[j].name, sn,
                                (size_t)sl) == 0) {
                            snv[j].value = sv;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        // Add to static storage
                        void *nn = realloc(
                            sd->static_named,
                            (size_t)
                            (sd->static_named_count + 1)
                            * sizeof(NamedVariable));
                        if (nn) {
                            NamedVariable *nv;
                            sd->static_named = nn;
                            nv = (NamedVariable *)nn;
                            nv += sd->static_named_count;
                            memcpy(nv->name, sn,
                                (size_t)sl);
                            nv->name[sl] = '\0';
                            nv->value = sv;
                            sd->static_named_count++;
                        }
                    }
                }
            }
        }
    }
}
