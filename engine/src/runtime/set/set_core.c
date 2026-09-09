// FILENAME: set_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: libcore (hal.h, memops.h, strops.h), libkernel (set.h)
// Implements core lifecycle and mutation operations for Set and Group in BASIC++.

#include "runtime/set.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/num_format.h"
#include "hal/hal.h"

// Release a single value stored inside a set or group
static void val_release_internal(void *str_ctx, BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_release((StringContext *)str_ctx, val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_release(str_ctx, val.as.map);
    } else if (val.type == VAL_SET && val.as.set) {
        set_release(str_ctx, val.as.set);
    } else if (val.type == VAL_GROUP && val.as.group) {
        group_release(str_ctx, val.as.group);
    }
}

// Retain a single value stored inside a set or group
static void val_retain_internal(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_add_ref(val.as.map);
    } else if (val.type == VAL_SET && val.as.set) {
        set_add_ref(val.as.set);
    } else if (val.type == VAL_GROUP && val.as.group) {
        group_add_ref(val.as.group);
    }
}

// ---- Group Implementation ----

BppGroup *group_create(const char *name) {
    HalContext *hal = hal_get();
    BppGroup *g = NULL;
    if (hal && hal->mem.alloc) {
        g = (BppGroup *)hal->mem.alloc(sizeof(BppGroup));
    }
    if (!g) return NULL;
    runtime_memset(g, 0, sizeof(BppGroup));
    g->ref_count = 1;
    g->capacity = 8;
    g->count = 0;
    if (name) {
        size_t nlen = runtime_strlen(name);
        if (hal && hal->mem.alloc) {
            g->name = (char *)hal->mem.alloc(nlen + 1);
            if (g->name) {
                runtime_memcpy(g->name, name, nlen + 1);
            }
        }
    }
    if (hal && hal->mem.alloc) {
        g->entries = (BppGroupEntry *)hal->mem.alloc((size_t)g->capacity * sizeof(BppGroupEntry));
    }
    if (!g->entries) {
        if (g->name && hal && hal->mem.free) hal->mem.free(g->name);
        if (hal && hal->mem.free) hal->mem.free(g);
        return NULL;
    }
    runtime_memset(g->entries, 0, (size_t)g->capacity * sizeof(BppGroupEntry));
    return g;
}

void group_add_ref(BppGroup *group) {
    if (group) group->ref_count++;
}

void group_release(void *str_ctx, BppGroup *group) {
    if (!group) return;
    group->ref_count--;
    if (group->ref_count <= 0) {
        HalContext *hal = hal_get();
        for (int i = 0; i < group->count; ++i) {
            if (group->entries[i].name && hal && hal->mem.free) {
                hal->mem.free(group->entries[i].name);
            }
            val_release_internal(str_ctx, group->entries[i].val);
        }
        if (hal && hal->mem.free) {
            if (group->name) hal->mem.free(group->name);
            hal->mem.free(group->entries);
            hal->mem.free(group);
        }
    }
}

bool group_set(void *str_ctx, BppGroup *group, const char *name, BValue val) {
    if (!group || !name) return false;
    HalContext *hal = hal_get();

    for (int i = 0; i < group->count; ++i) {
        if (group->entries[i].name && runtime_strcasecmp(group->entries[i].name, name) == 0) {
            val_release_internal(str_ctx, group->entries[i].val);
            group->entries[i].val = val;
            val_retain_internal(val);
            return true;
        }
    }

    if (group->count >= group->capacity) {
        int new_cap = group->capacity * 2;
        if (new_cap < 8) new_cap = 8;
        if (!hal || !hal->mem.alloc) return false;
        BppGroupEntry *new_entries = (BppGroupEntry *)hal->mem.alloc((size_t)new_cap * sizeof(BppGroupEntry));
        if (!new_entries) return false;
        runtime_memset(new_entries, 0, (size_t)new_cap * sizeof(BppGroupEntry));
        runtime_memcpy(new_entries, group->entries, (size_t)group->count * sizeof(BppGroupEntry));
        if (hal->mem.free) hal->mem.free(group->entries);
        group->entries = new_entries;
        group->capacity = new_cap;
    }

    size_t nlen = runtime_strlen(name);
    char *kcopy = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(nlen + 1) : NULL;
    if (!kcopy) return false;
    runtime_memcpy(kcopy, name, nlen + 1);

    group->entries[group->count].name = kcopy;
    group->entries[group->count].val = val;
    val_retain_internal(val);
    group->count++;
    return true;
}

bool group_get(BppGroup *group, const char *name, BValue *out_val) {
    if (!group || !name || !out_val) return false;
    for (int i = 0; i < group->count; ++i) {
        if (group->entries[i].name && runtime_strcasecmp(group->entries[i].name, name) == 0) {
            *out_val = group->entries[i].val;
            return true;
        }
    }
    return false;
}

bool group_get_at(BppGroup *group, int index, const char **out_name, BValue *out_val) {
    if (!group || index < 1 || index > group->count || !out_val) return false;
    if (out_name) *out_name = group->entries[index - 1].name;
    *out_val = group->entries[index - 1].val;
    return true;
}

bool group_has(BppGroup *group, const char *name) {
    if (!group || !name) return false;
    for (int i = 0; i < group->count; ++i) {
        if (group->entries[i].name && runtime_strcasecmp(group->entries[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

int group_count(BppGroup *group) {
    return group ? group->count : 0;
}

// ---- Set Implementation ----

BppSet *set_create(int capacity, bool is_bounded) {
    HalContext *hal = hal_get();
    BppSet *s = NULL;
    if (hal && hal->mem.alloc) {
        s = (BppSet *)hal->mem.alloc(sizeof(BppSet));
    }
    if (!s) return NULL;
    runtime_memset(s, 0, sizeof(BppSet));
    s->ref_count = 1;
    s->is_bounded = is_bounded ? 1 : 0;
    s->capacity = (capacity > 0) ? capacity : 8;
    s->count = 0;
    if (hal && hal->mem.alloc) {
        s->items = (BValue *)hal->mem.alloc((size_t)s->capacity * sizeof(BValue));
    }
    if (!s->items) {
        if (hal && hal->mem.free) hal->mem.free(s);
        return NULL;
    }
    runtime_memset(s->items, 0, (size_t)s->capacity * sizeof(BValue));
    return s;
}

void set_add_ref(BppSet *set) {
    if (set) set->ref_count++;
}

void set_release(void *str_ctx, BppSet *set) {
    if (!set) return;
    set->ref_count--;
    if (set->ref_count <= 0) {
        HalContext *hal = hal_get();
        for (int i = 0; i < set->count; ++i) {
            val_release_internal(str_ctx, set->items[i]);
        }
        if (hal && hal->mem.free) {
            hal->mem.free(set->items);
            hal->mem.free(set);
        }
    }
}

bool set_add(void *str_ctx, BppSet *set, BValue val) {
    (void)str_ctx;
    if (!set) return false;
    if (set_contains(set, val)) return true;
    HalContext *hal = hal_get();

    if (set->count >= set->capacity) {
        if (set->is_bounded) {
            return false; // Deterministic bound exceeded: Error 7 (Out of memory)
        }
        int new_cap = set->capacity * 2;
        if (new_cap < 8) new_cap = 8;
        if (!hal || !hal->mem.alloc) return false;
        BValue *new_items = (BValue *)hal->mem.alloc((size_t)new_cap * sizeof(BValue));
        if (!new_items) return false;
        runtime_memset(new_items, 0, (size_t)new_cap * sizeof(BValue));
        runtime_memcpy(new_items, set->items, (size_t)set->count * sizeof(BValue));
        if (hal->mem.free) hal->mem.free(set->items);
        set->items = new_items;
        set->capacity = new_cap;
    }

    set->items[set->count] = val;
    val_retain_internal(val);
    set->count++;
    return true;
}

bool set_get(BppSet *set, int index, BValue *out_val) {
    if (!set || index < 1 || index > set->count || !out_val) return false;
    *out_val = set->items[index - 1];
    return true;
}

bool set_set(void *str_ctx, BppSet *set, int index, BValue val) {
    if (!set || index < 1 || index > set->count) return false;
    val_release_internal(str_ctx, set->items[index - 1]);
    set->items[index - 1] = val;
    val_retain_internal(val);
    return true;
}

int set_count(BppSet *set) {
    return set ? set->count : 0;
}

bool set_remove_at(void *str_ctx, BppSet *set, int index) {
    if (!set || index < 1 || index > set->count) return false;
    val_release_internal(str_ctx, set->items[index - 1]);
    for (int i = index - 1; i < set->count - 1; ++i) {
        set->items[i] = set->items[i + 1];
    }
    set->count--;
    return true;
}
