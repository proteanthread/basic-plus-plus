// FILENAME: file_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (file.c, file_channel.c, file_io.c, file_record.c)
// NEEDED BY: libcore (file_txn.c)
// NEEDS: libcore, libkernel, libplatform
// Provides core logic and interface definitions for file_internal within BASIC++.
//
// ---- Includes ----

#ifndef FILE_INTERNAL_H
#define FILE_INTERNAL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal/hal.h"
#include "hal/io_hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

#include "debug/logger.h"
#include "device/vdev.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "security/security.h"
#include "types/config.h"
#include "types/errors.h"
#include "types/types.h"

//
// ---- Buffer Configuration ----

#define FILE_PAGE_BUFFER_SIZE 65536
#define STREAM_CHUNK_SIZE     262144
#define BASIC_MAX_RANGE_LOCKS 64

//
// ---- Internal Data Structures ----

typedef struct {
    IoHandle       handle;
    VDev          *vdev;
    char           filename[256];
    BppFileMode    mode;
    BppFileAccess  access;
    BppFileLockMode lock_mode;
    int            record_len;
    int            pushback_char;
    unsigned char *record_buffer;
    char          *page_buffer;
    char          *stream_buf;
    size_t         stream_pos;
    size_t         stream_len;
    size_t         write_accum_len;
} BppFileChannel;


typedef struct {
    int  channel;
    char filename[256];
    long start;
    long end;
    bool active;
} BppGlobalLock;

struct FileContext {
    MemoryContext  *mem;
    BppFileChannel  channels[BASIC_MAX_OPEN_FILES];
    BppGlobalLock   locks[BASIC_MAX_RANGE_LOCKS];

    int             txn_mode;
    BppTxnEntry     txn_journal[64];
    int             txn_entry_count;
    char            txn_journal_file[256];
    bool            txn_use_file;

    long            recount;
};

#endif // FILE_INTERNAL_H
