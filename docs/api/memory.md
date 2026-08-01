# Memory Manager API Reference

Header File: [`include/bpp_memory.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_memory.h)

## Overview
Implements raw allocation tracing, double-free protection, heap size controls, and debugging bounds.

## Exposed API Entities
### Structs & Types
- `MemoryContext MemoryContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `mem_shutdown` | `void` | `MemoryContext *ctx` |
| `mem_scratch_reset` | `void` | `MemoryContext *ctx` |
| `mem_program_insert` | `bool` | `MemoryContext *ctx, BppLineNumber line, const char *text` |
| `mem_program_delete` | `bool` | `MemoryContext *ctx, BppLineNumber line` |
| `mem_program_clear` | `void` | `MemoryContext *ctx` |
| `mem_lib_program_insert` | `bool` | `MemoryContext *ctx, BppLineNumber line, const char *text` |
| `mem_lib_program_clear` | `void` | `MemoryContext *ctx` |
| `mem_string_free` | `void` | `MemoryContext *ctx, void *ptr` |
| `mem_get_free_ram` | `size_t` | `MemoryContext *ctx` |
| `mem_get_used_ram` | `size_t` | `MemoryContext *ctx` |
| `mem_format_size` | `void` | `size_t bytes, char *buf, size_t buf_size` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_memory.h"

void alloc_mem() {
    void *ptr = mem_alloc_tracked(128, "MySubsystem");
    (void)ptr;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
