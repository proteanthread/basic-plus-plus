<!--
Title:        Freestanding Script and File I/O Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Freestanding Script and File I/O Architecture

## 1. Architectural Overview

The BASIC++ script runner and file I/O subsystem (`libscript`) encapsulates batch script execution, file channels, random-access record buffers, range locking, and transaction journaling under strict ISO C17 freestanding compliance (§4 ¶6).

All direct dependencies on hosted standard C file streams (`FILE*`), standard headers (`<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`), heap allocators, and string formatting have been replaced with the freestanding Hardware Abstraction Layer handle abstraction (`IoHandle`), `hal->io.*` operations, and the freestanding runtime library (`runtime_*`).

---

## 2. Subsystem Architecture & Source Inventory

### 2.1 File I/O Subsystem (`libscript`)
- **Public Handle Interface ([`engine/include/runtime/file.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/include/runtime/file.h))**: Replaces hosted `<stdio.h>` streams with `"hal/io_hal.h"`, exposing `file_get_handle` returning abstract `IoHandle`.
- **Channel Descriptors ([`engine/include/runtime/file_internal.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/include/runtime/file_internal.h))**: Stores channels as internal `IoHandle handle` structures with zero hosted libc inclusions.
- **Channel Lifecycle ([`engine/src/runtime/file/file_channel.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/file/file_channel.c))**: Channel allocation, opening, closing, and deletion (`file_init`, `file_shutdown`, `file_open`, `file_close`, `file_is_open`, `file_get_handle`) route to `hal->io.file_open`, `hal->io.file_close`, `hal->io.file_remove`, and `hal->mem.alloc`/`free`.
- **Sequential & Stream I/O ([`engine/src/runtime/file/file_io.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/file/file_io.c))**: Character, line, and block operations (`file_getc`, `file_ungetc`, `file_gets`, `file_puts`, `file_write_raw`, `file_printf`, `file_flush`, `file_read`, `file_write`) route to `hal->io.file_read`, `hal->io.file_write`, `hal->io.file_flush`, and `runtime_vsnprintf`.
- **Positioning & Status ([`engine/src/runtime/file/file_record.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/file/file_record.c))**: Position and file length querying (`file_lof`, `file_loc`, `file_eof`, `file_seek`, `file_get_status`, `file_get_typ`) route to `hal->io.file_seek`, `hal->io.file_tell`, and `hal->io.file_eof`.
- **Transactions & Range Locking ([`engine/src/runtime/file/file_txn.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/file/file_txn.c))**: Range locks (`file_lock_range`, `file_unlock_range`, `file_check_overlap`) and atomic rollback journaling (`file_txn_begin`, `file_txn_log_write`, `file_txn_rollback`, `file_txn_commit`) route through HAL operations.

### 2.2 Caller Synchronization
- **File Catalog ([`engine/src/statements/filesystem/files.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/filesystem/files.c))**: Rewinds channels via `file_seek(fc, ch, 1)` and `file_flush(fc, ch)` without exposing raw file descriptors.
- **Input File Operations ([`engine/src/statements/filesystem/input_file.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/filesystem/input_file.c))**: Uses freestanding `-1` EOF sentinels.
- **Matrix Stream Operations ([`engine/src/statements/matrices/mat_input.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/matrices/mat_input.c), `mat_read.c`)**: Uses freestanding EOF sentinels for structured matrix reading.

---

## 3. Verification & Test Suite

1. **Unit Test Suite ([`tests/script_freestanding_test.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tests/script_freestanding_test.c))**:
   - Sequential text I/O and channel lifecycle (`file_init`, `file_open`, `file_puts`, `file_gets`, `file_eof`, `file_close`).
   - Binary and random access I/O (`file_open`, `file_write`, `file_seek`, `file_loc`, `file_lof`, `file_read`).
   - Range locking and conflict detection (`file_lock_range`, `file_check_overlap`, `file_unlock_range`).
   - Transaction journaling and rollback (`file_txn_begin`, `file_txn_log_write`, `file_txn_rollback`, `file_txn_commit`).
2. **Freestanding Suite Execution**: All six freestanding suites pass with 100% verification.
3. **Master Regression Coverage**:
   - `tests/qb_vbdos_master.bas`: 10/10 Packages PASSED (100%).
   - `tests/vintage_ecosystems_master.bas`: 14/14 Packages PASSED (100%).
   - `tests/vintage_deep_fuzz_stress.bas`: 8/8 Tests PASSED (100%).
   - Batch runner execution: `bs -c "PRINT 42"` verified with exit code 0.
