// FILENAME: map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (map.h)
// NEEDS: libkernel (config.h)
// Provides core logic and interface definitions for map within BASIC++.
//
// ---- Includes ----

#include "runtime/map.h"
#include "runtime/strings.h"
#include "types/config.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

BppMap *map_create(void) {
    HalContext *hal = hal_get();
    BppMap *map = NULL;
    if (hal && hal->mem.alloc) {
        map = (BppMap *)hal->mem.alloc(sizeof(BppMap));
    }
    if (!map) return NULL;
    runtime_memset(map, 0, sizeof(BppMap));
    map->ref_count = 1;
    map->capacity = 8;
    map->count = 0;
    map->last_index = -1;
    if (hal && hal->mem.alloc) {
        map->entries = (BppMapEntry *)hal->mem.alloc((size_t)map->capacity * sizeof(BppMapEntry));
    }
    if (!map->entries) {
        if (hal && hal->mem.free) hal->mem.free(map);
        return NULL;
    }
    runtime_memset(map->entries, 0, (size_t)map->capacity * sizeof(BppMapEntry));
    return map;
}

void map_add_ref(BppMap *map) {
    if (map) {
        map->ref_count++;
    }
}

void map_release(void *str_ctx, BppMap *map) {
    if (!map) return;
    map->ref_count--;
    if (map->ref_count <= 0) {
        HalContext *hal = hal_get();
        for (int i = 0; i < map->count; ++i) {
            if (map->entries[i].key && hal && hal->mem.free) {
                hal->mem.free(map->entries[i].key);
            }
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                map_release(str_ctx, map->entries[i].val.as.map);
            }
        }
        if (hal && hal->mem.free) {
            hal->mem.free(map->entries);
            hal->mem.free(map);
        }
    }
}

bool map_set(void *str_ctx, BppMap *map, const char *key, BValue val) {
    if (!map || !key) return false;
    HalContext *hal = hal_get();

    // MRU fast-path
    if (map->last_index >= 0 && map->last_index < map->count) {
        int i = map->last_index;
        if (map->entries[i].key && runtime_strcasecmp(map->entries[i].key, key) == 0) {
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                map_release(str_ctx, map->entries[i].val.as.map);
            }
            map->entries[i].val = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            } else if (val.type == VAL_MAP && val.as.map) {
                map_add_ref(val.as.map);
            }
            return true;
        }
    }

    // Search for existing key case-insensitively
    for (int i = 0; i < map->count; ++i) {
        if (runtime_strcasecmp(map->entries[i].key, key) == 0) {
            map->last_index = i;
            // Release old value
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                map_release(str_ctx, map->entries[i].val.as.map);
            }
            // Set new value
            map->entries[i].val = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            } else if (val.type == VAL_MAP && val.as.map) {
                map_add_ref(val.as.map);
            }
            return true;
        }
    }

    // Grow if needed
    if (map->count >= map->capacity) {
        int old_cap = map->capacity;
        int new_cap = map->capacity * 2;
        BppMapEntry *new_entries = NULL;
        if (hal && hal->mem.realloc) {
            new_entries = (BppMapEntry *)hal->mem.realloc(map->entries, (size_t)new_cap * sizeof(BppMapEntry));
        } else if (hal && hal->mem.alloc) {
            new_entries = (BppMapEntry *)hal->mem.alloc((size_t)new_cap * sizeof(BppMapEntry));
            if (new_entries && map->entries) {
                runtime_memcpy(new_entries, map->entries, (size_t)old_cap * sizeof(BppMapEntry));
                if (hal->mem.free) hal->mem.free(map->entries);
            }
        }
        if (!new_entries) return false;
        // Zero-initialize newly allocated entries to ensure key=NULL sentinel
        runtime_memset(new_entries + map->capacity, 0, (size_t)(new_cap - map->capacity) * sizeof(BppMapEntry));
        map->entries = new_entries;
        map->capacity = new_cap;
    }

    // Copy key string
    size_t k_len = runtime_strlen(key);
    char *k_copy = NULL;
    if (hal && hal->mem.alloc) {
        k_copy = (char *)hal->mem.alloc(k_len + 1);
    }
    if (!k_copy) return false;
    runtime_memcpy(k_copy, key, k_len + 1);

    map->entries[map->count].key = k_copy;
    map->entries[map->count].val = val;
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        map_add_ref(val.as.map);
    }
    map->last_index = map->count;
    map->count++;
    return true;
}

bool map_get(BppMap *map, const char *key, BValue *out_val) {
    if (!map || !key || !out_val) return false;

    // MRU fast-path
    if (map->last_index >= 0 && map->last_index < map->count) {
        int i = map->last_index;
        if (map->entries[i].key && runtime_strcasecmp(map->entries[i].key, key) == 0) {
            *out_val = map->entries[i].val;
            return true;
        }
    }

    for (int i = 0; i < map->count; ++i) {
        if (map->entries[i].key && runtime_strcasecmp(map->entries[i].key, key) == 0) {
            map->last_index = i;
            *out_val = map->entries[i].val;
            return true;
        }
    }
    return false;
}

bool map_remove(void *str_ctx, BppMap *map, const char *key) {
    if (!map || !key) return false;
    HalContext *hal = hal_get();
    for (int i = 0; i < map->count; ++i) {
        if (runtime_strcasecmp(map->entries[i].key, key) == 0) {
            // Release key and value
            if (map->entries[i].key && hal && hal->mem.free) {
                hal->mem.free(map->entries[i].key);
            }
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                map_release(str_ctx, map->entries[i].val.as.map);
            }
            // Shift remainder down
            for (int j = i; j < map->count - 1; ++j) {
                map->entries[j] = map->entries[j + 1];
            }
            map->count--;
            // Zero out the vacated slot to maintain key=NULL invariant
            runtime_memset(&map->entries[map->count], 0, sizeof(BppMapEntry));
            return true;
        }
    }
    return false;
}

int map_count(BppMap *map) {
    return map ? map->count : 0;
}

const char *map_key(BppMap *map, int index) {
    if (!map || index < 0 || index >= map->count) return NULL;
    return map->entries[index].key;
}

bool map_has(BppMap *map, const char *key) {
    if (!map || !key) return false;
    for (int i = 0; i < map->count; ++i) {
        if (runtime_strcasecmp(map->entries[i].key, key) == 0) {
            return true;
        }
    }
    return false;
}

