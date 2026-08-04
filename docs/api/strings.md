# Strings Subsystem API Reference

Header File: [`include/strings.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/strings.h)

## Overview
Implements the internal string pool, garbage collection, and utility string functions.

## Exposed API Entities
### Structs & Types
- `StringContext StringContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `str_init` | `StringContext *` | `MemoryContext *mem` |
| `str_shutdown` | `void` | `StringContext *ctx` |
| `str_create` | `BppStringRef` | `StringContext *ctx, const char *data, size_t length` |
| `str_concat` | `BppStringRef` | `StringContext *ctx, BppStringRef a, BppStringRef b` |
| `str_mid` | `BppStringRef` | `StringContext *ctx, BppStringRef ref, size_t start, size_t len` |
| `str_data` | `const char *` | `BppStringRef ref` |
| `str_len` | `size_t` | `BppStringRef ref` |
| `str_add_ref` | `void` | `BppStringRef ref` |
| `str_release` | `void` | `StringContext *ctx, BppStringRef ref` |
| `str_gc` | `void` | `StringContext *ctx` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "strings.h"

void test_str(VMContext *vm) {
    BppStringRef s = str_create(vm_get_str(vm), "Hello", 5);
    (void)s;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
