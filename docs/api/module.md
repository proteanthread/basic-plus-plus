# Module Loader API Reference

Header File: [`include/bpp_module.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_module.h)

## Overview
Implements validation, registration, activation, and loading pipeline of shared library plugins.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `module_system_init` | `void` | `void` |
| `module_count` | `int` | `void` |
| `module_register` | `int` | `const BppModuleInfo *info` |
| `module_activate` | `int` | `const char *name, void *rt` |
| `module_deactivate` | `int` | `const char *name` |
| `module_is_active` | `int` | `const char *name` |
| `module_is_loaded` | `int` | `int index` |
| `module_caps_string` | `void` | `unsigned int caps, char *buf, int buf_len` |
| `module_load_dynamic` | `int` | `struct VMContext *vm, const char *path` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_module.h"

void load_mod() {
    module_load_dynamic(NULL, "custom_plugin.dll");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
