/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Implements a standalone key-value hash map and serialization utilities.
 * Why it exists: Provides Ring 1 data structure utilities independent of VM context.
 * Why it works this way: Uses dynamic arrays with case-insensitive key lookup.
 * What can be changed: Internal storage array can be upgraded to hash bucket arrays.
 * What cannot be changed: BppMap public functions exposed via bpp_collections.h.
 * What to expect: Reliable key-value storage and JSON/XML/YAML/INI string conversion.
 * What to do if something breaks: Check key allocation and free routines.
 * Assumptions: Keys are 7-bit ASCII strings.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add BSON binary serialization support.
 * External extension hooks: Exposed via bpp_collections.h.
 */

#include "runtime/collections.h"
#include "platform/platform.h"
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
    (void)str_ctx;
    if (!map) return;
    map->ref_count--;
    if (map->ref_count <= 0) {
        for (int i = 0; i < map->count; ++i) {
            if (map->entries[i].key) free(map->entries[i].key);
        }
        free(map->entries);
        free(map);
    }
}
