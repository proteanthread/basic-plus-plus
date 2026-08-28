// FILENAME: map.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for map within BASIC++.
//
// ---- Includes ----

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
    int          last_index;
};

// Map lifecycle APIs
BppMap     *map_create(void);
void        map_add_ref(BppMap *map);
void        map_release(void *str_ctx, BppMap *map);

// Map modification/query APIs
bool        map_set(void *str_ctx, BppMap *map, const char *key, BValue val);
bool        map_get(BppMap *map, const char *key, BValue *out_val);
bool        map_remove(void *str_ctx, BppMap *map, const char *key);
int         map_count(BppMap *map);
const char *map_key(BppMap *map, int index);
bool        map_has(BppMap *map, const char *key);

// Serialization APIs
BppMap     *map_parse_json(void *str_ctx, const char *json);
char       *map_stringify_json(BppMap *map);

BppMap     *map_parse_xml(void *str_ctx, const char *xml);
char       *map_stringify_xml(BppMap *map);

BppMap     *map_parse_yaml(void *str_ctx, const char *yaml);
char       *map_stringify_yaml(BppMap *map);

BppMap     *map_parse_ini(void *str_ctx, const char *ini);
char       *map_stringify_ini(BppMap *map);

#endif // RUNTIME_MAP_H
