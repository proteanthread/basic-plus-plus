# Logging Infrastructure API Reference

Header File: [`include/bpp_logger.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_logger.h)

## Overview
Controls output formats, trace levels, assertions, debugging logs, and logs redirection.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `bpp_logger_init` | `bool` | `const char *log_path, const char *out_path` |
| `bpp_logger_close` | `void` | `void` |
| `bpp_log_info` | `void` | `const char *fmt, ...` |
| `bpp_log_warn` | `void` | `const char *fmt, ...` |
| `bpp_log_error` | `void` | `const char *fmt, ...` |
| `bpp_log_write_out` | `void` | `const char *buf, size_t len` |
| `bpp_logger_set_debug` | `void` | `bool debug` |
| `bpp_logger_is_debug` | `bool` | `void` |
| `bpp_logger_set_dry_run` | `void` | `bool dry_run` |
| `bpp_logger_is_dry_run` | `bool` | `void` |
| `bpp_logger_set_trace` | `void` | `bool trace` |
| `bpp_logger_is_trace` | `bool` | `void` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_logger.h"

void write_log() {
    bpp_log_info("Subsystem initialized");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
