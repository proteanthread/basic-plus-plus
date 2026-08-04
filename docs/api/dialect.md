# Dialect Subsystem API Reference

Header File: [`include/dialect.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/dialect.h)

## Overview
Maintains dialect definitions, configurations, keyword tables, and option sets.

## Exposed API Entities
### Structs & Types
- `BppDialect`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `dialect_free` | `void` | `BppDialect *d` |
| `dialect_load_from_map` | `bool` | `VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len` |
| `dialect_validate_map` | `bool` | `VMContext *vm, BppMap *map, char *err_buf, size_t err_len` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "dialect.h"

void configure_dialect(VMContext *vm) {
    dialect_free(vm);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
