/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ — a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * What it does: Public umbrella header for Ring 1 standalone data structures & utilities.
 * Why it exists: Aggregates map, crypto, string utilities, sorting, and using formatting libraries.
 * Why it works this way: Allows consumers to `#include "runtime/collections.h"` for all data utilities.
 * What can be changed: Add new standalone collection structure declarations.
 * What cannot be changed: BppMap and utility function signatures.
 * What to expect: Clean compilation independent of VM or interpreter engine.
 * What to do if something breaks: Check included standard library headers.
 * Assumptions: C17 compiler compliance.
 * Portability concerns: Strict C17 compliant, pure 7-bit ASCII.
 * Future expansions: Add dynamic vector and queue data structures.
 * External extension hooks: Shared across all BASIC++ target executables and plugins.
 */

#ifndef BPP_COLLECTIONS_H
#define BPP_COLLECTIONS_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"

/* Map Data Structures */
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

BppMap *bpp_map_create(void);
void    bpp_map_add_ref(BppMap *map);
void    bpp_map_release(void *str_ctx, BppMap *map);

#endif /* BPP_COLLECTIONS_H */
