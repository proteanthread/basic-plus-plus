// FILENAME: collections.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, map.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for collections within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_COLLECTIONS_H
#define RUNTIME_COLLECTIONS_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"

// Map Data Structures
typedef struct BppMapEntry {
    char   *key;
    BValue  val;
} BppMapEntry;

typedef struct BppMap {
    int          ref_count;
    int          count;
    int          capacity;
    BppMapEntry *entries;
} BppMap;

BppMap *map_create(void);
void    map_add_ref(BppMap *map);
void    map_release(void *str_ctx, BppMap *map);

#endif // RUNTIME_COLLECTIONS_H
