/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file map.c
 * @brief Reference-counted dictionary/map structure lifecycle and key-value management.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements creation, copying (ref counting), and destruction of maps,
 *   as well as setting, getting, and removing keys.
 * - Why it exists: Provides the runtime execution support for the VAL_MAP BValue type.
 * - Why it works this way: It maintains entries in a dynamic array. Lookups scan the array
 *   case-insensitively using strcasecmp (defined via bpp_config.h mapping to _stricmp on Windows).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal entry arrays can be replaced with a hash table for O(1) performance.
 * - What cannot be changed: Opaque BppMap struct layout without updating include/bpp_map.h.
 * - What to expect: Releasing a map will recursively release its string and map values.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Key strings are case-insensitive.
 * - Portability concerns: None. ANSI/ISO C17 compliant.
 */

#include "runtime/map.h"
#include "runtime/strings.h"
#include "types/config.h"
#include <stdlib.h>
#include <string.h>


BppMap *bpp_map_create(void) {
    BppMap *map = (BppMap *)calloc(1, sizeof(BppMap));
    if (!map) return NULL;
    map->ref_count = 1;
    map->capacity = 8;
    map->count = 0;
    map->entries = (BppMapEntry *)calloc((size_t)map->capacity, sizeof(BppMapEntry));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    return map;
}

void bpp_map_add_ref(BppMap *map) {
    if (map) {
        map->ref_count++;
    }
}

void bpp_map_release(void *str_ctx, BppMap *map) {
    if (!map) return;
    map->ref_count--;
    if (map->ref_count <= 0) {
        for (int i = 0; i < map->count; ++i) {
            free(map->entries[i].key);
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                bpp_map_release(str_ctx, map->entries[i].val.as.map);
            }
        }
        free(map->entries);
        free(map);
    }
}

bool bpp_map_set(void *str_ctx, BppMap *map, const char *key, BValue val) {
    if (!map || !key) return false;

    /* Search for existing key case-insensitively */
    for (int i = 0; i < map->count; ++i) {
        if (strcasecmp(map->entries[i].key, key) == 0) {
            /* Release old value */
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                bpp_map_release(str_ctx, map->entries[i].val.as.map);
            }
            /* Set new value */
            map->entries[i].val = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            } else if (val.type == VAL_MAP && val.as.map) {
                bpp_map_add_ref(val.as.map);
            }
            return true;
        }
    }

    /* Grow if needed */
    if (map->count >= map->capacity) {
        int new_cap = map->capacity * 2;
        BppMapEntry *new_entries = (BppMapEntry *)realloc(map->entries, (size_t)new_cap * sizeof(BppMapEntry));
        if (!new_entries) return false;
        /* Zero-initialize newly allocated entries to ensure key=NULL sentinel */
        memset(new_entries + map->capacity, 0, (size_t)(new_cap - map->capacity) * sizeof(BppMapEntry));
        map->entries = new_entries;
        map->capacity = new_cap;
    }

    /* Copy key string */
    char *k_copy = (char *)calloc(1, strlen(key) + 1);
    if (!k_copy) return false;
    memcpy(k_copy, key, strlen(key) + 1);

    map->entries[map->count].key = k_copy;
    map->entries[map->count].val = val;
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        bpp_map_add_ref(val.as.map);
    }
    map->count++;
    return true;
}

bool bpp_map_get(BppMap *map, const char *key, BValue *out_val) {
    if (!map || !key || !out_val) return false;
    for (int i = 0; i < map->count; ++i) {
        if (strcasecmp(map->entries[i].key, key) == 0) {
            *out_val = map->entries[i].val;
            return true;
        }
    }
    return false;
}

bool bpp_map_remove(void *str_ctx, BppMap *map, const char *key) {
    if (!map || !key) return false;
    for (int i = 0; i < map->count; ++i) {
        if (strcasecmp(map->entries[i].key, key) == 0) {
            /* Release key and value */
            free(map->entries[i].key);
            if (map->entries[i].val.type == VAL_STRING && map->entries[i].val.as.string) {
                str_release((StringContext *)str_ctx, map->entries[i].val.as.string);
            } else if (map->entries[i].val.type == VAL_MAP && map->entries[i].val.as.map) {
                bpp_map_release(str_ctx, map->entries[i].val.as.map);
            }
            /* Shift remainder down */
            for (int j = i; j < map->count - 1; ++j) {
                map->entries[j] = map->entries[j + 1];
            }
            map->count--;
            /* Zero out the vacated slot to maintain key=NULL invariant */
            memset(&map->entries[map->count], 0, sizeof(BppMapEntry));
            return true;
        }
    }
    return false;
}

int bpp_map_count(BppMap *map) {
    return map ? map->count : 0;
}

const char *bpp_map_key(BppMap *map, int index) {
    if (!map || index < 0 || index >= map->count) return NULL;
    return map->entries[index].key;
}

bool bpp_map_has(BppMap *map, const char *key) {
    if (!map || !key) return false;
    for (int i = 0; i < map->count; ++i) {
        if (strcasecmp(map->entries[i].key, key) == 0) {
            return true;
        }
    }
    return false;
}
