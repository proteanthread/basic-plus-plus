# Type System API Reference

Header File: [`include/bpp_types.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_types.h)

## Overview
Defines core value representation (BValue) for numbers, strings, arrays, and objects.

## Exposed API Entities
### Structs & Types
- `BppString BppString`
- `BppMap BppMap`
- `BppTypeRegistry BppTypeRegistry`

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_types.h"
#include <stdio.h>

void show_val(BValue val) {
    if (val.type == VAL_NUMBER) {
        printf("Number: %f\n", val.as.number);
    }
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
