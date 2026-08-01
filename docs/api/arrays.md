# Array Manager API Reference

Header File: [`include/bpp_arrays.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_arrays.h)

## Overview
Provides bounds-checking multidimensional arrays, DIM, and REDIM capabilities.

## Exposed API Entities
### Structs & Types
- `ArrayContext ArrayContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `arr_shutdown` | `void` | `ArrayContext *ctx` |
| `arr_clear_all` | `void` | `ArrayContext *ctx` |
| `arr_dim` | `BppError` | `ArrayContext *ctx, const char *name, int num_dims, const int *bounds` |
| `arr_set_type` | `void` | `ArrayContext *ctx, const char *name, ValueType type` |
| `arr_erase` | `bool` | `ArrayContext *ctx, const char *name` |
| `arr_exists` | `bool` | `ArrayContext *ctx, const char *name` |
| `arr_ubound` | `int` | `ArrayContext *ctx, const char *name, int dimension, bool *out_found` |
| `arr_set_option_base` | `void` | `ArrayContext *ctx, int base` |
| `arr_get_option_base` | `int` | `ArrayContext *ctx` |
| `arr_get_dimensions` | `int` | `ArrayContext *ctx, const char *name, int *out_bounds, int max_dims` |
| `arr_get_last_det` | `double` | `ArrayContext *ctx` |
| `arr_set_last_det` | `void` | `ArrayContext *ctx, double val` |
| `arr_serialize` | `bool` | `ArrayContext *ctx, void *fp` |
| `arr_deserialize` | `bool` | `ArrayContext *ctx, void *fp` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_arrays.h"

void create_arr(ArrayContext *ctx) {
    int bounds[2] = {10, 10};
    arr_dim(ctx, "MYARRAY", 2, bounds);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
