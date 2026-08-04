# Map Data Structure API Reference

Header File: [`include/runtime/map.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/runtime/map.h)

## Overview
Implements an optimized hash map/dictionary for modern associative collections in BASIC++.

## Exposed API Entities
### Structs & Types
- `BppMapEntry`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `map_add_ref` | `void` | `BppMap *map` |
| `map_release` | `void` | `void *str_ctx, BppMap *map` |
| `map_set` | `bool` | `void *str_ctx, BppMap *map, const char *key, BValue val` |
| `map_get` | `bool` | `BppMap *map, const char *key, BValue *out_val` |
| `map_remove` | `bool` | `void *str_ctx, BppMap *map, const char *key` |
| `map_count` | `int` | `BppMap *map` |
| `map_has` | `bool` | `BppMap *map, const char *key` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "runtime/map.h"

void run_map(VMContext *vm) {
    BppMap *m = map_create(vm);
    (void)m;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
