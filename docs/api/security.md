# Security Sandbox API Reference

Header File: [`include/security.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/security.h)

## Overview
Enforces permission checks, capabilities restrictions, and security policies on system calls.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `security_init` | `void` | `BppSecLevel level` |
| `security_get_level` | `BppSecLevel` | `void` |
| `security_set_level` | `void` | `BppSecLevel level` |
| `security_find_level_by_name` | `int` | `const char *name` |
| `security_check` | `int` | `BppSecOperation op, int line_num` |
| `security_module_allowed` | `int` | `unsigned int capabilities` |
| `security_check_pinned_level` | `int` | `BppSecLevel required_level` |
| `security_check_mem` | `int` | `unsigned long address, int size` |
| `security_check_port` | `int` | `int port, int line_num` |
| `security_check_path` | `int` | `const char *path, int line_num` |
| `security_check_file_path` | `int` | `const char *path, int line_num` |
| `security_restrict_op` | `int` | `BppSecOperation op` |
| `security_is_op_restricted` | `int` | `BppSecOperation op` |
| `security_restrict_keyword` | `int` | `int kw_id` |
| `security_is_keyword_restricted` | `int` | `int kw_id` |
| `security_restrict_list` | `void` | `void` |
| `security_restrict_count` | `int` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "security.h"

void check_sec() {
    if (security_check(SECOP_FILE_READ, 0) != 0) {
        // Access Denied
    }
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
