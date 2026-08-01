# Function Registry API Reference

Header File: [`include/bpp_funcreg.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_funcreg.h)

## Overview
Manages built-in functions, modular extensions registry, and DEF FN overrides.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `funcreg_init` | `void` | `void` |
| `funcreg_set_registering_module` | `void` | `const char *name` |
| `funcreg_register` | `int` | `const FunctionEntry *entry` |
| `funcreg_override` | `int` | `BppKeywordId kw, FuncHandler handler` |
| `funcreg_count` | `int` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_funcreg.h"

BValue custom_func(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(42.0);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
