// FILENAME: map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (collections.h, string.h)
// NEEDS: libengine (string.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for map within BASIC++.
//
// ---- Includes ----

#include "runtime/collections.h"
#include "platform/platform.h"
#include <stdlib.h>
#include <string.h>

BppMap *map_create(void) {
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

void map_add_ref(BppMap *map) {
    if (map) {
        map->ref_count++;
    }
}

void map_release(void *str_ctx, BppMap *map) {
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
