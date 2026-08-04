# File I/O Subsystem API Reference

Header File: [`include/file.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/file.h)

## Overview
Controls file descriptors, legacy channel mappings, binary, random-access, and sequential modes.

## Exposed API Entities
### Structs & Types
- `FileContext FileContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `file_shutdown` | `void` | `FileContext *ctx` |
| `file_open` | `BppError` | `FileContext *ctx, VDevContext *vdev_ctx, int channel, const char *filename, BppFileMode mode, BppFileAccess access, BppFileLockMode lock_mode, int record_len` |
| `file_lock_range` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_unlock_range` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_check_overlap` | `BppError` | `FileContext *ctx, int channel, long start, long end` |
| `file_close` | `void` | `FileContext *ctx, int channel` |
| `file_close_all` | `void` | `FileContext *ctx` |
| `file_is_open` | `bool` | `FileContext *ctx, int channel` |
| `file_get_mode` | `BppFileMode` | `FileContext *ctx, int channel` |
| `file_lof` | `long` | `FileContext *ctx, int channel` |
| `file_loc` | `long` | `FileContext *ctx, int channel` |
| `file_eof` | `bool` | `FileContext *ctx, int channel` |
| `file_seek` | `void` | `FileContext *ctx, int channel, long position` |
| `file_get_record_len` | `int` | `FileContext *ctx, int channel` |
| `file_getc` | `int` | `FileContext *ctx, int channel` |
| `file_putc` | `int` | `FileContext *ctx, int channel, int c` |
| `file_ungetc` | `int` | `FileContext *ctx, int channel, int c` |
| `file_puts` | `int` | `FileContext *ctx, int channel, const char *s` |
| `file_printf` | `int` | `FileContext *ctx, int channel, const char *fmt, ...` |
| `file_flush` | `int` | `FileContext *ctx, int channel` |
| `file_read` | `int` | `FileContext *ctx, int channel, void *buf, int len` |
| `file_write` | `int` | `FileContext *ctx, int channel, const void *buf, int len` |
| `file_txn_status` | `int` | `FileContext *ctx` |
| `file_txn_begin` | `void` | `FileContext *ctx, int mode, bool use_file` |
| `file_txn_commit` | `BppError` | `FileContext *ctx` |
| `file_txn_rollback` | `BppError` | `FileContext *ctx` |
| `file_txn_entry_count` | `int` | `FileContext *ctx` |
| `file_txn_log_write` | `void` | `FileContext *ctx, int channel, long position, const void *old_data, int len` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "file.h"

void open_file(FileContext *ctx, VDevContext *vdev_ctx) {
    file_open(ctx, vdev_ctx, 1, "data.txt", FILE_MODE_OUTPUT, FILE_ACCESS_DEFAULT, FILE_LOCK_DEFAULT, 0);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
