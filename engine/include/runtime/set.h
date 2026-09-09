// FILENAME: set.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for Set and Group in BASIC++.

#ifndef RUNTIME_SET_H
#define RUNTIME_SET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types/types.h"

// Group member entry representing a named or indexed field within a Group
typedef struct BppGroupEntry {
    char   *name;       // Member name (heap-allocated string, or NULL for purely ordinal member)
    BValue  val;        // Bound member value (scalar, group, or set)
} BppGroupEntry;

// Group data structure representing an addressable subset or record within a Set
struct BppGroup {
    int            ref_count;  // Reference count for deterministic lifecycle
    char          *name;       // Optional name of group (or NULL)
    int            count;      // Current number of members
    int            capacity;   // Allocated capacity
    BppGroupEntry *entries;    // Array of member entries
};

// Set data structure representing an ordered multiset or algebraic set
struct BppSet {
    int     ref_count;  // Reference count for deterministic lifecycle
    int     is_bounded; // 1 if fixed-capacity bounded (freestanding), 0 if arena/growable
    int     count;      // Current number of elements
    int     capacity;   // Total element capacity
    BValue *items;      // Contiguous array of elements
};

// ---- Group Lifecycle & Mutation APIs ----

BppGroup *group_create(const char *name);
void      group_add_ref(BppGroup *group);
void      group_release(void *str_ctx, BppGroup *group);
bool      group_set(void *str_ctx, BppGroup *group, const char *name, BValue val);
bool      group_get(BppGroup *group, const char *name, BValue *out_val);
bool      group_get_at(BppGroup *group, int index, const char **out_name, BValue *out_val);
bool      group_has(BppGroup *group, const char *name);
int       group_count(BppGroup *group);

// ---- Set Lifecycle & Mutation APIs ----

BppSet   *set_create(int capacity, bool is_bounded);
void      set_add_ref(BppSet *set);
void      set_release(void *str_ctx, BppSet *set);
bool      set_add(void *str_ctx, BppSet *set, BValue val);
bool      set_get(BppSet *set, int index, BValue *out_val);
bool      set_set(void *str_ctx, BppSet *set, int index, BValue val);
int       set_count(BppSet *set);
bool      set_remove_at(void *str_ctx, BppSet *set, int index);

// ---- Set & Group Algebraic Operations ----

bool      value_equals(BValue a, BValue b);
BppSet   *set_union(void *str_ctx, BppSet *a, BppSet *b);
BppSet   *set_intersection(void *str_ctx, BppSet *a, BppSet *b);
BppSet   *set_difference(void *str_ctx, BppSet *a, BppSet *b);
BppSet   *set_sym_diff(void *str_ctx, BppSet *a, BppSet *b);
bool      set_contains(BppSet *s, BValue val);
bool      set_is_subset(BppSet *a, BppSet *b);
bool      set_is_proper_subset(BppSet *a, BppSet *b);
int       set_cardinality(BppSet *s);

BppGroup *group_union(void *str_ctx, BppGroup *a, BppGroup *b);
BppGroup *group_intersection(void *str_ctx, BppGroup *a, BppGroup *b);
BppGroup *group_difference(void *str_ctx, BppGroup *a, BppGroup *b);
BppGroup *group_sym_diff(void *str_ctx, BppGroup *a, BppGroup *b);
bool      group_is_subset(BppGroup *a, BppGroup *b);
bool      group_is_proper_subset(BppGroup *a, BppGroup *b);

// ---- Map Subsumption Interop ----

BppGroup *group_from_map(void *str_ctx, struct BppMap *map);
struct BppMap *map_from_group(void *str_ctx, BppGroup *group);

// ---- Multi-Tier Path Addressing & Relational Projection ----

bool      group_path_get(void *str_ctx, BppGroup *group, int depth, BValue *keys, BValue *out_val);
bool      group_path_set(void *str_ctx, BppGroup *group, int depth, BValue *keys, BValue val);
bool      set_path_get(void *str_ctx, BValue root, int depth, BValue *keys, BValue *out_val);
bool      set_path_set(void *str_ctx, BValue *root, int depth, BValue *keys, BValue val);

BppSet   *set_project(void *str_ctx, BppSet *set, const char *field);
BppSet   *set_project_filter(void *str_ctx, BppSet *set, const char *filter_key, BValue filter_val, const char *field);

// ---- Pick MultiValue Dynamic Array Conversion & Ops ----

BppSet   *set_from_dynarray(void *str_ctx, const char *str);
char     *set_to_dynarray(void *str_ctx, BValue val);
BValue    set_dyn_extract(void *str_ctx, BValue target, int attr, int val, int subval, BppError *err);
BValue    set_dyn_replace(void *str_ctx, BValue target, int attr, int val, int subval, BValue new_val, BppError *err);
BValue    set_dyn_insert(void *str_ctx, BValue target, int attr, int val, int subval, BValue new_val, BppError *err);
BValue    set_dyn_delete(void *str_ctx, BValue target, int attr, int val, int subval, BppError *err);

// ---- Serialization & Formatting ----

bool      set_format(char *buf, size_t buf_size, BppSet *set);
bool      group_format(char *buf, size_t buf_size, BppGroup *group);

// ---- Container Unification Layer ----

bool      set_unify_array_get(void *vm, const char *name, int index, BValue *out_val, BppError *err);
bool      set_unify_array_set(void *vm, const char *name, int index, BValue val, BppError *err);

#endif // RUNTIME_SET_H
