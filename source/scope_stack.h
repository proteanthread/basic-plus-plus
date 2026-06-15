/*
 * ---
 * BASIC++ Interpreter - scope_stack.h
 * ---
 *
 * Dynamic scope stack for SUB/FUNCTION local variable isolation.
 *
 * Each CALL/DEFine PROCedure pushes a scope snapshot that saves
 * the caller's variable state. On END SUB / EXIT SUB / RETurn,
 * the snapshot is popped and the caller's variables are restored.
 *
 * Two scope modes are supported:
 *
 * SCOPE_FULL (0) - Full snapshot save/restore.
 *     All named variables are saved on entry and restored on exit.
 *     The SUB body inherits the caller's variable values.
 *     Used by: ECMA-116, Tymshare Super BASIC, B++ native.
 *
 * SCOPE_FRESH (1) - QBasic-style fresh scope.
 *     All named variables are saved on entry, then zeroed.
 *     The SUB body starts with a clean slate.
 *     Only SHARED variables carry through from the caller.
 *     Used by: QBasic, GWBS.
 *
 * Memory management:
 *     Named variable snapshots are dynamically allocated (malloc).
 *     The stack itself grows in chunks (SCOPE_CHUNK_SIZE bytes).
 *     Maximum depth is SCOPE_MAX_DEPTH levels of nesting.
 *
 * LOCAL save/restore:
 *     Each scope level has a local save list. When LOCal is
 *     executed inside a SUB, the listed variables' current values
 *     are saved on this list and then zeroed. On scope pop,
 *     LOCAL-saved values are restored before the full snapshot
 *     restore, ensuring LOCal variables revert properly.
 *
 * ---
 */

#ifndef BASICPP_SCOPE_STACK_H
#define BASICPP_SCOPE_STACK_H

#include "config.h"
#include "value.h"

/* Scope modes */
#define SCOPE_FULL   0  /* ECMA-116 / Super BASIC: full snapshot */
#define SCOPE_FRESH  1  /* QBasic: fresh zeroed vars */

/* Dynamic growth parameters */
#define SCOPE_INITIAL_DEPTH  16   /* initial allocated levels */
#define SCOPE_MAX_DEPTH      256  /* absolute max nesting */
#define SCOPE_CHUNK_SIZE     (64 * 1024)  /* 64KB growth unit */
#define SCOPE_MAX_BYTES      (512 * 1024) /* 512KB total cap */

/* LOCAL save entry (per-variable save within a scope level) */
#define SCOPE_MAX_LOCALS     32   /* max LOCal vars per scope level */

/* Forward declare RuntimeState to break circular dependency */
struct RuntimeState;

typedef struct LocalSave {
    char name[MAX_VAR_NAME_LEN + 1];
    int name_len;
    BValue value;
    int is_single_letter;  /* 1 = A-Z, 0 = named var */
    char var_letter;       /* if is_single_letter: 'A'-'Z' */
    int is_string_var;     /* 1 = A$-Z$ */
} LocalSave;

typedef struct ScopeSnapshot {
    /* Saved single-letter variables A-Z */
    BValue saved_vars[MAX_VARIABLES];
    /* Saved string variables A$-Z$ */
    BValue saved_strvars[MAX_STRING_VARS];
    /* Dynamically allocated copy of named variables */
    void *named_vars;     /* NamedVariable* - void* to avoid circular include */
    int named_count;
    /* Scope metadata */
    int scope_mode;       /* SCOPE_FULL or SCOPE_FRESH */
    int sub_index;        /* index into SubDef table */
    int return_index;     /* program index to return to after END SUB */
    /* LOCAL save list */
    LocalSave locals[SCOPE_MAX_LOCALS];
    int local_count;
    /* SHARED variable names (not saved/restored on pop) */
    char shared_names[SCOPE_MAX_LOCALS][MAX_VAR_NAME_LEN + 1];
    int shared_count;
    /* STATIC variable tracking (saved back to SubDef on pop) */
    char static_names[SCOPE_MAX_LOCALS][MAX_VAR_NAME_LEN + 1];
    int static_name_lens[SCOPE_MAX_LOCALS];
    char static_letters[SCOPE_MAX_LOCALS]; /* 'A'-'Z' or 0 */
    int static_is_string[SCOPE_MAX_LOCALS];
    int static_count;
    int all_static; /* 1 = SUB...STATIC: all vars are static */
} ScopeSnapshot;

typedef struct ScopeStack {
    ScopeSnapshot *levels;  /* dynamically allocated array */
    int depth;              /* current nesting depth (0 = no active scope) */
    int capacity;           /* allocated level count */
} ScopeStack;

/*
 * scope_stack_init - Initialize the scope stack.
 *
 * Allocates initial capacity. Must be called before any push/pop.
 */
void scope_stack_init(ScopeStack *ss);

/*
 * scope_stack_free - Free all scope stack memory.
 *
 * Frees all snapshots and the stack itself.
 * Call on runtime_reset or interpreter shutdown.
 */
void scope_stack_free(ScopeStack *ss);

/*
 * scope_stack_push - Push a new scope level.
 *
 * Saves the current variable state from RuntimeState into a new
 * snapshot. If mode == SCOPE_FRESH, zeros all variables after save.
 *
 * Parameters:
 *   ss        - scope stack
 *   rt        - runtime state (source of variables to save)
 *   mode      - SCOPE_FULL or SCOPE_FRESH
 *   sub_index - index of the SubDef being called
 *   return_idx - program index to return to on END SUB
 *
 * Returns 0 on success, -1 on error (stack full or malloc failure).
 */
int scope_stack_push(ScopeStack *ss, struct RuntimeState *rt,
                     int mode, int sub_index, int return_idx);

/*
 * scope_stack_pop - Pop the top scope level and restore variables.
 *
 * Restores LOCAL-saved variables first, then restores the full
 * snapshot. SHARED variables are NOT restored (changes propagate
 * back to the caller).
 *
 * Sets rt->next_index to the saved return_index.
 *
 * Parameters:
 *   ss - scope stack
 *   rt - runtime state (destination for restored variables)
 *
 * Returns 0 on success, -1 on error (stack empty).
 */
int scope_stack_pop(ScopeStack *ss, struct RuntimeState *rt);

/*
 * scope_stack_depth - Return current nesting depth.
 */
int scope_stack_depth(const ScopeStack *ss);

/*
 * scope_stack_add_local - Register a LOCal variable in current scope.
 *
 * Saves the variable's current value and zeros it.
 * On scope_stack_pop, the saved value will be restored.
 *
 * Parameters:
 *   ss   - scope stack
 *   rt   - runtime state
 *   name - variable name (NULL for single-letter A-Z)
 *   name_len - length of name (0 for single-letter)
 *   var_letter - 'A'-'Z' for single-letter vars, 0 for named
 *   is_string - 1 if this is a string variable (A$-Z$)
 *
 * Returns 0 on success, -1 if no active scope or locals full.
 */
int scope_stack_add_local(ScopeStack *ss, struct RuntimeState *rt,
                          const char *name, int name_len,
                          char var_letter, int is_string);

/*
 * scope_stack_add_shared - Mark a variable as SHARED in current scope.
 *
 * SHARED variables are not restored on scope pop — their changes
 * propagate back to the caller.
 *
 * Returns 0 on success, -1 if no active scope or list full.
 */
int scope_stack_add_shared(ScopeStack *ss, const char *name,
                           int name_len);

/*
 * scope_stack_add_static - Register a STATIC variable in current scope.
 *
 * STATIC variables persist between calls to the same SUB/FUNCTION.
 * On scope push, if the SUB has stored static data, those values
 * are restored instead of using the caller's values.
 * On scope pop, the STATIC variables' current values are saved
 * back to the SubDef's static storage.
 *
 * Parameters:
 *   ss         - scope stack
 *   rt         - runtime state
 *   name       - variable name (NULL for single-letter A-Z)
 *   name_len   - length of name (0 for single-letter)
 *   var_letter - 'A'-'Z' for single-letter vars, 0 for named
 *   is_string  - 1 if this is a string variable (A$-Z$)
 *
 * Returns 0 on success, -1 if no active scope or list full.
 */
int scope_stack_add_static(ScopeStack *ss, struct RuntimeState *rt,
                           const char *name, int name_len,
                           char var_letter, int is_string);

/*
 * scope_stack_save_static - Save static vars back to SubDef on scope exit.
 *
 * Called by scope_stack_pop before restoring the caller's variables.
 * Saves the current values of all STATIC-marked variables into
 * the SubDef's static storage so they persist to the next call.
 */
void scope_stack_save_static(ScopeStack *ss, struct RuntimeState *rt);

#endif /* BASICPP_SCOPE_STACK_H */
