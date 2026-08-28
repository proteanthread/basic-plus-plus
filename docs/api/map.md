# C17 API Reference: Associative Map Subsystem (`runtime/map.h`)

## 1. Subsystem Overview & Responsibilities

The Associative Map Subsystem (`runtime/map.h`, implemented in `engine/src/runtime/map.c`) provides reference-counted key-value dictionary collections, fast string key lookups, and built-in serialization (JSON, XML, YAML, INI) for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Reference-Counted Map Handles**: Enables maps to be passed by reference across BASIC variables, functions, and arrays with automatic refcount lifecycle management (`map_add_ref()`, `map_release()`).
- **Dynamic Key-Value Storage**: Stores string keys mapped to polymorphic tagged union `BValue` elements (numeric, string, or nested map objects).
- **Built-in Serialization & Parsing**:
  - `JSON`: `map_parse_json()`, `map_stringify_json()`.
  - `XML`: `map_parse_xml()`, `map_stringify_xml()`.
  - `YAML`: `map_parse_yaml()`, `map_stringify_yaml()`.
  - `INI`: `map_parse_ini()`, `map_stringify_ini()`.
- **String Memory Integration**: Coordinates with `StringContext` to retain and release string values stored inside map entries.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/map.h"
#include "types/types.h"
#include "runtime/strings.h"
```

## 3. Data Structures & Types

```c
/* Single Key-Value Map Entry */
typedef struct BppMapEntry {
    char   *key;    /* Null-terminated string key */
    BValue  val;    /* Associated BValue */
} BppMapEntry;

/* Reference-Counted Map Descriptor */
typedef struct BppMap {
    int          ref_count; /* Reference counter */
    BppMapEntry *entries;   /* Dynamic entry array */
    int          count;     /* Current key-value count */
    int          capacity;  /* Allocated entry capacity */
} BppMap;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle Management
```c
/**
 * @brief Creates a new empty associative map.
 */
BppMap *map_create(void);

/**
 * @brief Increments the map reference counter.
 */
void map_add_ref(BppMap *map);

/**
 * @brief Decrements the refcount and deallocates the map when it reaches 0.
 * @param str_ctx String context used to release string values.
 */
void map_release(void *str_ctx, BppMap *map);
```

### Key-Value Manipulation & Querying
```c
bool        map_set(void *str_ctx, BppMap *map, const char *key, BValue val);
bool        map_get(BppMap *map, const char *key, BValue *out_val);
bool        map_remove(void *str_ctx, BppMap *map, const char *key);
int         map_count(BppMap *map);
const char *map_key(BppMap *map, int index);
bool        map_has(BppMap *map, const char *key);
```

### Serialization & Parsing
```c
BppMap *map_parse_json(void *str_ctx, const char *json);
char   *map_stringify_json(BppMap *map);

BppMap *map_parse_xml(void *str_ctx, const char *xml);
char   *map_stringify_xml(BppMap *map);

BppMap *map_parse_yaml(void *str_ctx, const char *yaml);
char   *map_stringify_yaml(BppMap *map);

BppMap *map_parse_ini(void *str_ctx, const char *ini);
char   *map_stringify_ini(BppMap *map);
```

## 5. Architectural Invariants

- **Memory Cleanup**: Releasing a map releases all contained `VAL_STRING` values via `str_release()` and frees all key strings.
- **Key Uniqueness**: `map_set()` replaces existing values for duplicate keys without creating orphaned entries.

## 6. Code Example: Parsing JSON into Map and Accessing Values

```c
#include "runtime/map.h"
#include <stdio.h>

void json_demo(StringContext *str_ctx) {
    const char *json = "{\"name\": \"Alice\", \"age\": 30}";
    BppMap *map = map_parse_json(str_ctx, json);
    if (map) {
        BValue age_val;
        if (map_get(map, "age", &age_val) && age_val.type == VAL_NUMBER) {
            printf("Age: %f\n", age_val.as.number);
        }
        map_release(str_ctx, map);
    }
}
```
