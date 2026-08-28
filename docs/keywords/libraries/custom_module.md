# `custom_module` Custom Extension Module Template

## 1. Architectural Purpose & Overview

The `custom_module` template illustrates how to structure, compile, and export dynamic shared modules (`.dll` / `.so`) for BASIC++ v6.5.2.

### Implementation Blueprint (C17):
```c
#include "module/module.h"
#include "runtime/funcreg.h"

int bpp_module_init(VMContext *vm) {
    // Register custom statements and functions
    return 0; // Success
}

void bpp_module_shutdown(VMContext *vm) {
    // Teardown module allocations
}
```
