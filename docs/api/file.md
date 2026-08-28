# C17 API Reference: Filesystem & Channel Runtime (`runtime/file.h`)

## 1. Subsystem Overview & Responsibilities

The Filesystem & Channel Runtime Subsystem (`runtime/file.h`, implemented in `engine/src/runtime/file.c`) manages open file channels (`1` to `BASIC_MAX_OPEN_FILES`), sequential stream I/O, random-access record buffers, binary byte streams, byte-range record locks (`LOCK`/`UNLOCK`), and transaction journals for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Unified Channel Abstraction**: Maps numeric channels (1..256) to standard host disk files, memory buffers, or virtual devices (`VDev *`).
- **File Modes & Access Control**: Supports `FILE_MODE_INPUT`, `FILE_MODE_OUTPUT`, `FILE_MODE_APPEND`, `FILE_MODE_BINARY`, and `FILE_MODE_RANDOM` with access flags (`FILE_ACCESS_READ`, `FILE_ACCESS_WRITE`, `FILE_ACCESS_READ_WRITE`).
- **GW-BASIC / QuickBASIC Random Record Buffering**: Allocates and pins fixed-size `record_buffer` arrays for `FIELD`, `GET`, `PUT`, `LSET`, and `RSET` operations.
- **Byte-Range Concurrency Locking**: Maintains up to 64 active range locks (`BppGlobalLock`) coordinated with platform file locking primitives.
- **Pushback & EOF Tracking**: Guarantees deterministic end-of-stream detection via `file_eof()` and single-character pushback (`file_ungetc()`).

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/file.h"
#include "device/vdev.h"
#include "memory/memory.h"
```

## 3. Data Structures & Types

```c
typedef enum {
    FILE_MODE_INPUT,
    FILE_MODE_OUTPUT,
    FILE_MODE_APPEND,
    FILE_MODE_BINARY,
    FILE_MODE_RANDOM
} BppFileMode;

typedef enum {
    FILE_ACCESS_DEFAULT     = 0,
    FILE_ACCESS_READ        = 1,
    FILE_ACCESS_WRITE       = 2,
    FILE_ACCESS_READ_WRITE  = 3
} BppFileAccess;

typedef enum {
    FILE_LOCK_DEFAULT       = 0,
    FILE_LOCK_SHARED        = 1,
    FILE_LOCK_READ          = 2,
    FILE_LOCK_WRITE         = 3,
    FILE_LOCK_READ_WRITE    = 4
} BppFileLockMode;

/* Opaque Handle to File Context */
typedef struct FileContext FileContext;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle & Channel Management
```c
FileContext *file_init(MemoryContext *mem);
void         file_shutdown(FileContext *ctx);

/**
 * @brief Opens a file or device on a designated channel (1..256).
 */
BppError file_open(
    FileContext    *ctx,
    VDevContext    *vdev_ctx,
    int             channel,
    const char     *filename,
    BppFileMode     mode,
    BppFileAccess   access,
    BppFileLockMode lock_mode,
    int             record_len
);

/**
 * @brief Closes an individual file channel or all open channels.
 */
void file_close(FileContext *ctx, int channel);
void file_close_all(FileContext *ctx);

/**
 * @brief Checks if a channel is currently open.
 */
bool file_is_open(FileContext *ctx, int channel);
```

### Stream & Record I/O Operations
```c
bool     file_eof(FileContext *ctx, int channel);
long     file_loc(FileContext *ctx, int channel);
uint64_t file_lof(FileContext *ctx, int channel);
bool     file_seek(FileContext *ctx, int channel, long byte_offset);

int      file_getc(FileContext *ctx, int channel);
int      file_putc(FileContext *ctx, int channel, int c);
void     file_ungetc(FileContext *ctx, int channel, int c);

size_t   file_read(FileContext *ctx, int channel, void *buf, size_t count);
size_t   file_write(FileContext *ctx, int channel, const void *buf, size_t count);
void     file_flush(FileContext *ctx, int channel);
```

### Random-Access Buffering & Locking
```c
unsigned char *file_get_record_buffer(FileContext *ctx, int channel);
int            file_get_record_len(FileContext *ctx, int channel);

bool file_lock_range(FileContext *ctx, int channel, long start_offset, long end_offset);
bool file_unlock_range(FileContext *ctx, int channel, long start_offset, long end_offset);
```

## 5. Architectural Invariants

- **Channel Boundary Protection**: Channel indices $< 1$ or $> 256$ immediately return Error 52 (`ERR_BAD_FILE_NUMBER`).
- **Resource Durability**: All dirty stream buffers are flushed before descriptors are released during `file_close()` and `file_close_all()`.

## 6. Code Example: Writing and Reading a Sequential Stream in C

```c
#include "runtime/file.h"

void file_demo(FileContext *fc, VDevContext *vdev) {
    /* Open channel #1 for output */
    BppError err = file_open(fc, vdev, 1, "DEMO.DAT", FILE_MODE_OUTPUT, FILE_ACCESS_WRITE, FILE_LOCK_DEFAULT, 128);
    if (err.code == 0) {
        const char *msg = "BASIC++ Runtime I/O\n";
        file_write(fc, 1, msg, strlen(msg));
        file_close(fc, 1);
    }
}
```
