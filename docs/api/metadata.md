# Metadata Subsystem API Reference

Header File: [`include/metadata.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/metadata.h)

## Overview
Handles compiler annotations, pragmas, directives, and metadata headers.

## Exposed API Entities
### Structs & Types
- `VMContext VMContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `metadata_init` | `void` | `BppMetadataRegistry *reg` |
| `metadata_register_label` | `bool` | `BppMetadataRegistry *reg, const char *label, const char *filename, BppLineNumber line` |
| `metadata_resolve_label` | `bool` | `const BppMetadataRegistry *reg, const char *label, char *out_filename, size_t max_len, BppLineNumber *out_line` |
| `metadata_register_docstring` | `bool` | `BppMetadataRegistry *reg, const char *target, const char *docstring` |
| `metadata_register_block` | `bool` | `BppMetadataRegistry *reg, const char *type, const char *target, const char *docstring, const char *body` |
| `metadata_pre_scan_line` | `void` | `VMContext *vm, const char *filename, BppLineNumber line_num, const char *text` |
| `metadata_pre_scan_program` | `void` | `VMContext *vm, const char *filename` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "metadata.h"

void print_meta(VMContext *vm) {
    (void)vm;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
