# Variables Subsystem API Reference

Header File: [`include/bpp_variables.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_variables.h)

## Overview
Manages variables, lexical scoping, globals, and lookups.

## Exposed API Entities
### Structs & Types
- `VariableContext VariableContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `var_shutdown` | `void` | `VariableContext *ctx` |
| `var_assign` | `bool` | `VariableContext *ctx, const char *name, BValue val` |
| `var_clear_all` | `void` | `VariableContext *ctx` |
| `var_clear_scope` | `void` | `VariableContext *ctx, const char *prefix` |
| `var_set_scope` | `void` | `VariableContext *ctx, const char *scope` |
| `var_set_shared` | `void` | `VariableContext *ctx, const char *name` |
| `var_set_explicit` | `void` | `VariableContext *ctx, bool enable` |
| `var_is_explicit` | `bool` | `VariableContext *ctx` |
| `var_set_namespace` | `void` | `VariableContext *ctx, const char *ns` |
| `var_set_case_sensitive` | `void` | `VariableContext *ctx, bool enable` |
| `var_set_def_type` | `void` | `VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type` |
| `var_get_def_type` | `ValueType` | `VariableContext *ctx, const char *scope, char letter` |
| `var_print_all` | `void` | `VariableContext *ctx, void *vdev_ptr` |
| `var_serialize` | `bool` | `VariableContext *ctx, void *fp` |
| `var_deserialize` | `bool` | `VariableContext *ctx, void *fp` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_variables.h"

void set_var(VMContext *vm) {
    BValue val = bval_float(42.0);
    var_assign(vm, "MYVAR", val);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
