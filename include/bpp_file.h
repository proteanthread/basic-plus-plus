/**
 * @file bpp_file.h
 * @brief Portable File System Channel manager API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares interfaces for opening, closing, reading, writing, and
 *   seeking file channels (1 to 16) conforming to classic QBASIC/GW-BASIC standards.
 * - Why it exists: Decouples statement executors from raw C stdio file operations,
 *   enabling capability checking and path virtualization inside sandbox limits.
 * - Why it works this way: It manages a registry table of open file channels. Channels are
 *   tracked by index (1-based) rather than raw pointers.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Maximum open file limits (BPP_MAX_OPEN_FILES).
 * - What cannot be changed: Virtual channel indexes mapping and bounds.
 * - What to expect: Accessing unopened channels returns BppError.
 * - What to do if something breaks: If file corruption occurs, check record length alignment.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: File names are standard null-terminated strings. Modes match standard C fopen options.
 * - Portability concerns: C17 stdio handles are universally available.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add remote file virtualizers (HTTP:, FTP:) by registering hook handlers.
 * - How to write external extensions: Plugins write to open files using channel descriptors.
 */

#ifndef BPP_FILE_H
#define BPP_FILE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "bpp_types.h"
#include "bpp_memory.h"
#include "bpp_vdev.h"

typedef enum {
    FILE_MODE_INPUT,
    FILE_MODE_OUTPUT,
    FILE_MODE_APPEND,
    FILE_MODE_BINARY,
    FILE_MODE_RANDOM
} BppFileMode;

typedef enum {
    FILE_ACCESS_DEFAULT = 0,
    FILE_ACCESS_READ = 1,
    FILE_ACCESS_WRITE = 2,
    FILE_ACCESS_READ_WRITE = 3
} BppFileAccess;

typedef enum {
    FILE_LOCK_DEFAULT = 0,
    FILE_LOCK_SHARED = 1,
    FILE_LOCK_READ = 2,
    FILE_LOCK_WRITE = 3,
    FILE_LOCK_READ_WRITE = 4
} BppFileLockMode;


typedef struct {
    int   channel;
    long  position;
    int   len;
    unsigned char *data;
} BppTxnEntry;

/* Opaque File System Channel Context */
typedef struct FileContext FileContext;

/**
 * @brief Initialize and shutdown the file context.
 */
FileContext *file_init(MemoryContext *mem);
void         file_shutdown(FileContext *ctx);

/**
 * @brief Open a file or device on a specific channel.
 */
BppError     file_open(FileContext *ctx, VDevContext *vdev_ctx, int channel, const char *filename, BppFileMode mode, BppFileAccess access, BppFileLockMode lock_mode, int record_len);

/**
 * @brief Lock a specific byte/record range for a channel.
 */
BppError     file_lock_range(FileContext *ctx, int channel, long start, long end);

/**
 * @brief Unlock a specific byte/record range for a channel.
 */
BppError     file_unlock_range(FileContext *ctx, int channel, long start, long end);

/**
 * @brief Check if a read/write operation overlaps with an exclusive lock from another channel.
 */
BppError     file_check_overlap(FileContext *ctx, int channel, long start, long end);

/**
 * @brief Close a specific file channel.
 */
void         file_close(FileContext *ctx, int channel);


/**
 * @brief Get the record buffer for a random access file channel.
 */
unsigned char *file_get_record_buffer(FileContext *ctx, int channel);

/**
 * @brief Close all currently active channels.
 */
void         file_close_all(FileContext *ctx);

/**
 * @brief Query if a specific channel is active.
 */
bool         file_is_open(FileContext *ctx, int channel);

/**
 * @brief Access standard C FILE handle for a channel.
 */
FILE        *file_get_handle(FileContext *ctx, int channel);

/**
 * @brief Get the open mode for a channel.
 */
BppFileMode  file_get_mode(FileContext *ctx, int channel);

/**
 * @brief Query length of file (LOF).
 */
long         file_lof(FileContext *ctx, int channel);

/**
 * @brief Query current position offset (LOC).
 */
long         file_loc(FileContext *ctx, int channel);

/**
 * @brief Query if channel has reached end-of-file.
 */
bool         file_eof(FileContext *ctx, int channel);

/**
 * @brief Seek to a specific byte offset.
 */
void         file_seek(FileContext *ctx, int channel, long position);

/**
 * @brief Query record length.
 */
int          file_get_record_len(FileContext *ctx, int channel);

/* Channel stream abstraction helpers (Phase 16) */
VDev        *file_get_vdev(FileContext *ctx, int channel);
int          file_getc(FileContext *ctx, int channel);
int          file_putc(FileContext *ctx, int channel, int c);
int          file_ungetc(FileContext *ctx, int channel, int c);
char        *file_gets(FileContext *ctx, int channel, char *buf, size_t size);
int          file_puts(FileContext *ctx, int channel, const char *s);
int          file_printf(FileContext *ctx, int channel, const char *fmt, ...);
int          file_flush(FileContext *ctx, int channel);
int          file_read(FileContext *ctx, int channel, void *buf, int len);
int          file_write(FileContext *ctx, int channel, const void *buf, int len);

/* Transaction APIs */
int          file_txn_status(FileContext *ctx);
void         file_txn_begin(FileContext *ctx, int mode, bool use_file);
BppError     file_txn_commit(FileContext *ctx);
BppError     file_txn_rollback(FileContext *ctx);
int          file_txn_entry_count(FileContext *ctx);
void         file_txn_log_write(FileContext *ctx, int channel, long position, const void *old_data, int len);

#endif /* BPP_FILE_H */
