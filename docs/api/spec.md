# Language Specification API Reference

Header File: [`include/spec.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/spec.h)

## Overview
Queries compiler capability layers and dialect compatibility definitions.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `spec_registry_init` | `void` | `void` |
| `spec_load_file` | `int` | `VMContext *vm, const char *filename` |
| `spec_load_companion_libraries` | `int` | `VMContext *vm, const char *dir_part` |
| `spec_register_inline` | `int` | `VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level` |
| `spec_get_count` | `int` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "spec.h"

void check_spec() {
    SpecObject *obj = spec_find_by_name("MYKEYWORD");
    (void)obj;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
