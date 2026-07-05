/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: scope_stack_core.h
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
 * File: scope_stack_core.h
 * ===================================================================== */

#ifndef STANDALONE_SCOPE_STACK_CORE_H
#define STANDALONE_SCOPE_STACK_CORE_H

#include "config.h"
#include "value.h"

// Scope modes
#define SCOPE_FULL   0
#define SCOPE_FRESH  1

// Dynamic growth parameters
#define SCOPE_INITIAL_DEPTH  16
#define SCOPE_MAX_DEPTH      256
#define SCOPE_CHUNK_SIZE     (64 * 1024)
#define SCOPE_MAX_BYTES      (512 * 1024)

// LOCAL save entry (per-variable save within a scope level)
#define SCOPE_MAX_LOCALS     32

typedef struct LocalSave {
    char name[MAX_VAR_NAME_LEN + 1];
    int name_len;
    BValue value;
    int is_single_letter; // 1 = A-Z, 0 = named var
    char var_letter; // if is_single_letter: 'A'-'Z'
    int is_string_var; // 1 = A$-Z$
} LocalSave;

typedef struct ScopeSnapshot {
    // Saved single-letter variables A-Z
    BValue saved_vars[MAX_VARIABLES];
    // Saved string variables A$-Z$
    BValue saved_strvars[MAX_STRING_VARS];
    // Dynamically allocated copy of named variables
    void *named_vars; // void* to avoid circular dependency
    int named_count;
    // Scope metadata
    int scope_mode; // SCOPE_FULL or SCOPE_FRESH
    int sub_index; // index into SubDef table
    int return_index; // program index to return to after END SUB
    // LOCAL save list
    LocalSave locals[SCOPE_MAX_LOCALS];
    int local_count;
    // SHARED variable names (not saved/restored on pop)
    char shared_names[SCOPE_MAX_LOCALS][MAX_VAR_NAME_LEN + 1];
    int shared_count;
    // STATIC variable tracking (saved back to SubDef on pop)
    char static_names[SCOPE_MAX_LOCALS][MAX_VAR_NAME_LEN + 1];
    int static_name_lens[SCOPE_MAX_LOCALS];
    char static_letters[SCOPE_MAX_LOCALS]; // 'A'-'Z' or 0
    int static_is_string[SCOPE_MAX_LOCALS];
    int static_count;
    int all_static; // 1 = SUB...STATIC: all vars are static
} ScopeSnapshot;

typedef struct ScopeStack {
    ScopeSnapshot *levels; // dynamically allocated array
    int depth; // current nesting depth (0 = no active scope)
    int capacity; // allocated level count
} ScopeStack;

/* Callbacks for named variable operations during push/pop/local */
typedef BValue (*ScopeStackGetNamedFn)(void *user_data, const char *name, int name_len);
typedef void (*ScopeStackSetNamedFn)(void *user_data, const char *name, int name_len, BValue val);

void scope_stack_core_init(ScopeStack *ss);
void scope_stack_core_free(ScopeStack *ss);
int scope_stack_core_grow(ScopeStack *ss);

int scope_stack_core_push(ScopeStack *ss, 
                          const BValue *variables, 
                          const BValue *string_vars,
                          const void *named_vars_src, 
                          int named_count,
                          size_t named_var_size,
                          int mode, int sub_index, int return_idx);

int scope_stack_core_pop(ScopeStack *ss,
                         BValue *variables,
                         BValue *string_vars,
                         void *named_vars_dest,
                         int *named_count_ptr,
                         size_t named_var_size,
                         ScopeStackGetNamedFn get_named_cb,
                         ScopeStackSetNamedFn set_named_cb,
                         void *cb_user_data);

int scope_stack_core_add_local(ScopeStack *ss,
                               BValue *variables,
                               BValue *string_vars,
                               ScopeStackGetNamedFn get_named_cb,
                               ScopeStackSetNamedFn set_named_cb,
                               void *cb_user_data,
                               const char *name, int name_len,
                               char var_letter, int is_string);

int scope_stack_core_add_shared(ScopeStack *ss, const char *name, int name_len);

#endif /* STANDALONE_SCOPE_STACK_CORE_H */
