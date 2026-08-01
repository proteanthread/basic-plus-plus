# Array Extensions module API Reference

Header File: [`include/mod_arrayext.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/mod_arrayext.h)

## Overview
Implements high performance vector operations and array math extensions.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `arrayext_execute_map` | `BppError` | `VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str` |
| `arrayext_func_map` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_filter` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_reduce` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `arrayext_func_aggregate` | `BValue` | `VMContext *vm, LexerContext *lex, int agg_type, BppError *err` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "mod_arrayext.h"

void run_arrayext() {
    // Array extensions
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
