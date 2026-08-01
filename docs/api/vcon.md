# Virtual Console API Reference

Header File: [`include/bpp_vcon.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_vcon.h)

## Overview
Manages standard character I/O, cursor controls, print formatting, and escape sequences.

## Exposed API Entities
### Structs & Types
- `VConContext VConContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vcon_shutdown` | `void` | `VConContext *ctx` |
| `vcon_select` | `bool` | `VConContext *ctx, int index` |
| `vcon_get_active_index` | `int` | `VConContext *ctx` |
| `vcon_write_char` | `void` | `VConContext *ctx, int index, int c` |
| `vcon_write_string` | `void` | `VConContext *ctx, int index, const char *s` |
| `vcon_clear` | `void` | `VConContext *ctx, int index` |
| `vcon_get_cursor` | `void` | `VConContext *ctx, int index, int *row, int *col` |
| `vcon_get_char_at` | `int` | `VConContext *ctx, int index, int row, int col` |
| `vcon_get_attr_at` | `int` | `VConContext *ctx, int index, int row, int col` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_vcon.h"

void print_con(VMContext *vm) {
    vcon_write(vm, (const uint8_t *)"Hello", 5);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
