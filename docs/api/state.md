# State Persistence API Reference

Header File: [`include/state.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/state.h)

## Overview
Saves and loads virtual machine state context structures.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vm_state_save` | `BppError` | `VMContext *vm, const char *filename` |
| `vm_state_load` | `BppError` | `VMContext *vm, const char *filename` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "state.h"

void save_state(VMContext *vm) {
    (void)vm;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
