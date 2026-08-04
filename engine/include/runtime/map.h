/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file runtime/map.h
 * @brief Reference-counted dictionary/map structure and serialization.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures and lifecycle APIs for dynamic key-value maps
 *   and serialization formats (JSON, XML, YAML, INI).
 * - Why it exists: Provides modern structured data types to BASIC++ to support APIs,
 *   configuration parsing, and structured data handling.
 * - Why it works this way: It uses an array of BppMapEntry records. Key-value lookups
 *   are case-insensitive. Map values are reference-counted to manage nested structures safely.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal hash table implementation or search algorithms.
 * - What cannot be changed: Opaque BppMap handle definition and signature of map functions.
 * - What to expect: Maps can contain numbers, strings, or other nested maps.
 * - What to do if something breaks: Trace ref_count increments and releases to isolate leaks.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Keys are case-insensitive null-terminated C strings.
 * - Portability concerns: ANSI/ISO C17 compliant.
 */

#ifndef RUNTIME_MAP_H
#define RUNTIME_MAP_H

#include "types/types.h"
#include <stdbool.h>

typedef struct BppMapEntry {
    char   *key;
    BValue  val;
} BppMapEntry;

struct BppMap {
    int          ref_count;
    BppMapEntry *entries;
    int          count;
    int          capacity;
};

/* Map lifecycle APIs */
BppMap     *map_create(void);
void        map_add_ref(BppMap *map);
void        map_release(void *str_ctx, BppMap *map);

/* Map modification/query APIs */
bool        map_set(void *str_ctx, BppMap *map, const char *key, BValue val);
bool        map_get(BppMap *map, const char *key, BValue *out_val);
bool        map_remove(void *str_ctx, BppMap *map, const char *key);
int         map_count(BppMap *map);
const char *map_key(BppMap *map, int index);
bool        map_has(BppMap *map, const char *key);

/* Serialization APIs */
BppMap     *map_parse_json(void *str_ctx, const char *json);
char       *map_stringify_json(BppMap *map);

BppMap     *map_parse_xml(void *str_ctx, const char *xml);
char       *map_stringify_xml(BppMap *map);

BppMap     *map_parse_yaml(void *str_ctx, const char *yaml);
char       *map_stringify_yaml(BppMap *map);

BppMap     *map_parse_ini(void *str_ctx, const char *ini);
char       *map_stringify_ini(BppMap *map);

#endif /* RUNTIME_MAP_H */
