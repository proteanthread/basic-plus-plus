# Segmented Memory (RAMBANK) API Reference

Header File: [`include/segmented_mem.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/segmented_mem.h)

## Overview
Mimics legacy banked/segmented memory architectures using virtual page lookups.

## Exposed API Entities
### Structs & Types
- `VMemContext VMemContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vmem_shutdown` | `void` | `VMemContext *ctx` |
| `vmem_set_def_seg` | `void` | `VMemContext *ctx, uint16_t seg` |
| `vmem_get_def_seg` | `uint16_t` | `VMemContext *ctx` |
| `vmem_register_handle` | `uint32_t` | `VMemContext *ctx, BValue *val, bool is_string_data` |
| `vmem_peek` | `int` | `VMemContext *ctx, uint16_t address, uint8_t *out_val` |
| `vmem_poke` | `int` | `VMemContext *ctx, uint16_t address, uint8_t val` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "segmented_mem.h"

void read_bank(VMemContext *ctx) {
    uint8_t val = 0;
    vmem_peek(ctx, 0x4000, &val);
    (void)val;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
