# Logging Infrastructure API Reference

Header File: [`include/logger.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/logger.h)

## Overview
Controls output formats, trace levels, assertions, debugging logs, and logs redirection.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `logger_init` | `bool` | `const char *log_path, const char *out_path` |
| `logger_close` | `void` | `void` |
| `log_info` | `void` | `const char *fmt, ...` |
| `log_warn` | `void` | `const char *fmt, ...` |
| `log_error` | `void` | `const char *fmt, ...` |
| `log_write_out` | `void` | `const char *buf, size_t len` |
| `logger_set_debug` | `void` | `bool debug` |
| `logger_is_debug` | `bool` | `void` |
| `logger_set_dry_run` | `void` | `bool dry_run` |
| `logger_is_dry_run` | `bool` | `void` |
| `logger_set_trace` | `void` | `bool trace` |
| `logger_is_trace` | `bool` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "logger.h"

void write_log() {
    log_info("Subsystem initialized");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
