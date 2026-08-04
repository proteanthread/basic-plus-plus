# User-Defined Structures API Reference

Header File: [`include/struct_ctx.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/struct_ctx.h)

## Overview
Implements custom structured types (TYPE ... END TYPE) in the runtime.

## Exposed API Entities
### Structs & Types
- `BppTypeRegistry`
- `VMContext VMContext`
- `BppMap BppMap`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `struct_registry_init` | `void` | `BppTypeRegistry *reg` |
| `struct_register_type` | `bool` | `BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len` |
| `struct_copy_instance` | `bool` | `VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "struct_ctx.h"

void run_struct(VMContext *vm) {
    (void)vm;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
