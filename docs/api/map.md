# Map Data Structure API Reference

Header File: [`include/bpp_map.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_map.h)

## Overview
Implements an optimized hash map/dictionary for modern associative collections in BASIC++.

## Exposed API Entities
### Structs & Types
- `BppMapEntry`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `bpp_map_add_ref` | `void` | `BppMap *map` |
| `bpp_map_release` | `void` | `void *str_ctx, BppMap *map` |
| `bpp_map_set` | `bool` | `void *str_ctx, BppMap *map, const char *key, BValue val` |
| `bpp_map_get` | `bool` | `BppMap *map, const char *key, BValue *out_val` |
| `bpp_map_remove` | `bool` | `void *str_ctx, BppMap *map, const char *key` |
| `bpp_map_count` | `int` | `BppMap *map` |
| `bpp_map_has` | `bool` | `BppMap *map, const char *key` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_map.h"

void run_map(VMContext *vm) {
    BppMap *m = map_create(vm);
    (void)m;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
