/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file file.c
 * @brief Portable File System Channel manager implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements opening, closing, seeking, and status queries on files
 *   mapped to channels 1 to 16.
 * - Why it exists: Abstracts raw file pointers from statement runners, enabling
 *   uniform file access on modern and classic operating systems.
 * - Why it works this way: It uses an array of structures on the heap containing
 *   host FILE* handles and configuration flags, normalized to 1-based channel indexes.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Mapping rules for fopen options (e.g. read/write permissions).
 * - What cannot be changed: 1-based index conversion calculations (index = channel - 1).
 * - What to expect: Unclosed files are automatically closed on context shutdown.
 * - What to do if something breaks: Check that files are opened in binary mode if raw
 *   record offsets are utilized.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Host filesystem supports seeking and standard file descriptors.
 * - Portability concerns: fopen, fclose, fseek, ftell are part of ANSI C.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add virtual path resolving inside file_open.
 * - How to write external extensions: Custom virtual block devices query channel handles.
 */

#include "runtime/file.h"
#include "types/config.h"
#include "security/security.h"
#include "debug/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
    FILE       *fp;
    VDev       *vdev;         /* Phase 16: device pointer (or NULL) */
    char        filename[256];
    BppFileMode mode;
    BppFileAccess access;
    BppFileLockMode lock_mode;
    int         record_len;
    int         pushback_char; /* -1 if empty */
    unsigned char *record_buffer; /* Phase 16: GW-BASIC Random Access File Buffer */
} BppFileChannel;

#define BASIC_MAX_RANGE_LOCKS 64

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

    /* Transaction log state */
    int             txn_mode;          /* 0 = none, 1 = explicit (TXN), 2 = implicit (ATOMIC) */
    BppTxnEntry     txn_journal[64];   /* Journal array */
    int             txn_entry_count;   /* Journal entry count */
    char            txn_journal_file[256]; /* Path to temporary journal file if file-based */
    bool            txn_use_file;      /* true = file-based, false = in-memory */
};

FileContext *file_init(MemoryContext *mem) {
    if (!mem) return NULL;
    FileContext *ctx = (FileContext *)calloc(1, sizeof(FileContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        ctx->channels[i].fp = NULL;
        ctx->channels[i].vdev = NULL;
        ctx->channels[i].filename[0] = '\0';
        ctx->channels[i].pushback_char = -1;
    }
    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        ctx->locks[i].active = false;
    }
    ctx->txn_mode = 0;
    ctx->txn_entry_count = 0;
    ctx->txn_journal_file[0] = '\0';
    ctx->txn_use_file = false;
    for (int i = 0; i < 64; ++i) {
        ctx->txn_journal[i].channel = 0;
        ctx->txn_journal[i].position = 0;
        ctx->txn_journal[i].len = 0;
        ctx->txn_journal[i].data = NULL;
    }
    return ctx;
}

void file_shutdown(FileContext *ctx) {
    if (!ctx) return;
    file_close_all(ctx);
    for (int i = 0; i < 64; ++i) {
        if (ctx->txn_journal[i].data) {
            free(ctx->txn_journal[i].data);
            ctx->txn_journal[i].data = NULL;
        }
    }
    if (ctx->txn_use_file && ctx->txn_journal_file[0]) {
        remove(ctx->txn_journal_file);
    }
    free(ctx);
}

BppError file_open(FileContext *ctx, VDevContext *vdev_ctx, int channel, const char *filename, BppFileMode mode, BppFileAccess access, BppFileLockMode lock_mode, int record_len) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (channel < 1 || channel > BASIC_MAX_OPEN_FILES) {
        err.code = 52; /* Bad file number */
        err.message = "Bad file number";
        return err;
    }

    int idx = channel - 1;
    if (ctx->channels[idx].fp != NULL || ctx->channels[idx].vdev != NULL) {
        err.code = 55; /* File already open */
        err.message = "File already open";
        return err;
    }

    /* Device Detection: Check if prefix before first colon matches a registered device */
    VDev *dev = NULL;
    const char *colon = strchr(filename, ':');
    if (colon && vdev_ctx) {
        char dev_name[256];
        size_t name_len = colon - filename + 1;
        if (name_len < sizeof(dev_name)) {
            memcpy(dev_name, filename, name_len);
            dev_name[name_len] = '\0';
            dev = vdev_get(vdev_ctx, dev_name);
        }
    }

    if (dev) {
        /* Gate 2: File operation security check (virtual device access) */
        if (security_check(SECOP_VDEV, 0) != 0) {
            err.code = 70; /* Permission Denied */
            err.message = "Permission denied: virtual device access restricted";
            return err;
        }

        /* Gate 1: Module capability check against security level */
        if (!security_module_allowed(dev->dev_req_caps)) {
            err.code = 70; /* Permission Denied */
            err.message = "Permission denied: device capabilities restricted";
            return err;
        }

        /* Call lifecycle open hook if available */
        if (dev->dev_open) {
            const char *path_param = colon + 1;
            int open_res = dev->dev_open(dev, path_param, mode);
            if (open_res < 0) {
                err.code = 57; /* Device I/O error / open failed */
                err.message = "Device open failed";
                return err;
            }
        }

        ctx->channels[idx].vdev = dev;
        ctx->channels[idx].fp = NULL;
        strncpy(ctx->channels[idx].filename, filename, sizeof(ctx->channels[idx].filename) - 1);
        ctx->channels[idx].filename[sizeof(ctx->channels[idx].filename) - 1] = '\0';
        ctx->channels[idx].mode = mode;
        ctx->channels[idx].access = access;
        ctx->channels[idx].lock_mode = lock_mode;
        ctx->channels[idx].record_len = (record_len > 0) ? record_len : BASIC_DEFAULT_RECORD_LEN;
        ctx->channels[idx].pushback_char = -1;
        if (mode == FILE_MODE_RANDOM) {
            ctx->channels[idx].record_buffer = (unsigned char *)calloc(1, ctx->channels[idx].record_len);
            if (!ctx->channels[idx].record_buffer) {
                err.code = 7; err.message = "Out of memory in file_open";
                return err;
            }
        } else {
            ctx->channels[idx].record_buffer = NULL;
        }

        return err;
    }

    /* Fallback: Regular File path */
    /* Enforce CWD-relative paths when security is enabled */
    if (security_get_level() >= SEC_STANDARD) {
        if (security_check_file_path(filename, 0) != 0) {
            err.code = 70; /* Permission Denied */
            err.message = "Permission denied: path access restricted";
            return err;
        }
    }

    /* Check for sharing violations across other open channels */
    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        if (i != idx && (ctx->channels[i].fp || ctx->channels[i].vdev)) {
            if (strcmp(ctx->channels[i].filename, filename) == 0) {
                BppFileLockMode existing_lock = ctx->channels[i].lock_mode;
                BppFileAccess existing_access = ctx->channels[i].access;
                /* If either one requires exclusive access and the other wants it */
                bool conflict = false;
                if (existing_lock == FILE_LOCK_READ && (access == FILE_ACCESS_READ || access == FILE_ACCESS_READ_WRITE || access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (existing_lock == FILE_LOCK_WRITE && (access == FILE_ACCESS_WRITE || access == FILE_ACCESS_READ_WRITE || access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (existing_lock == FILE_LOCK_READ_WRITE) conflict = true;
                
                if (lock_mode == FILE_LOCK_READ && (existing_access == FILE_ACCESS_READ || existing_access == FILE_ACCESS_READ_WRITE || existing_access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (lock_mode == FILE_LOCK_WRITE && (existing_access == FILE_ACCESS_WRITE || existing_access == FILE_ACCESS_READ_WRITE || existing_access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (lock_mode == FILE_LOCK_READ_WRITE) conflict = true;

                if (conflict) {
                    err.code = 70; /* Permission Denied */
                    err.message = "Permission denied (sharing violation)";
                    return err;
                }
            }
        }
    }

    const char *fopen_mode = "r";
    switch (mode) {
        case FILE_MODE_INPUT:  fopen_mode = "r";   break;
        case FILE_MODE_OUTPUT: fopen_mode = "w";   break;
        case FILE_MODE_APPEND: fopen_mode = "a";   break;
        case FILE_MODE_BINARY:
        case FILE_MODE_RANDOM: {
            /* Try opening for read/write update in binary */
            FILE *test = fopen(filename, "r+b");
            if (test) {
                fclose(test);
                fopen_mode = "r+b";
            } else {
                fopen_mode = "w+b";
            }
            break;
        }
    }

    const char *open_target = filename;
    if (logger_is_dry_run()) {
        bool is_write = false;
        if (mode == FILE_MODE_OUTPUT || mode == FILE_MODE_APPEND) {
            is_write = true;
        } else if (access == FILE_ACCESS_WRITE || access == FILE_ACCESS_READ_WRITE || access == FILE_ACCESS_DEFAULT) {
            is_write = true;
        }
        if (is_write) {
            log_warn("[DRY-RUN] Intercepted mutating file open for: %s (redirected to null stream)", filename);
#if defined(_WIN32)
            open_target = "NUL";
#else
            open_target = "/dev/null";
#endif
        }
    }

    FILE *fp = fopen(open_target, fopen_mode);
    if (!fp) {
        err.code = 53; /* File not found / permission error */
        err.message = "File not found or access denied";
        return err;
    }

    ctx->channels[idx].fp = fp;
    ctx->channels[idx].vdev = NULL;
    strncpy(ctx->channels[idx].filename, filename, sizeof(ctx->channels[idx].filename) - 1);
    ctx->channels[idx].filename[sizeof(ctx->channels[idx].filename) - 1] = '\0';
    ctx->channels[idx].mode = mode;
    ctx->channels[idx].access = access;
    ctx->channels[idx].lock_mode = lock_mode;
    ctx->channels[idx].record_len = (record_len > 0) ? record_len : BASIC_DEFAULT_RECORD_LEN;
    ctx->channels[idx].pushback_char = -1;
    if (mode == FILE_MODE_RANDOM) {
        ctx->channels[idx].record_buffer = (unsigned char *)calloc(1, ctx->channels[idx].record_len);
        if (!ctx->channels[idx].record_buffer) {
            if (fp) fclose(fp);
            ctx->channels[idx].fp = NULL;
            err.code = 7; err.message = "Out of memory in file_open";
            return err;
        }
    } else {
        ctx->channels[idx].record_buffer = NULL;
    }

    return err;
}

void file_close(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        fclose(chan->fp);
        chan->fp = NULL;
    } else if (chan->vdev) {
        if (chan->vdev->dev_close) {
            chan->vdev->dev_close(chan->vdev);
        }
        chan->vdev = NULL;
    }

    if (chan->record_buffer) {
        free(chan->record_buffer);
        chan->record_buffer = NULL;
    }

    /* Release any range locks held by this channel */
    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && ctx->locks[i].channel == channel) {
            ctx->locks[i].active = false;
        }
    }

    chan->filename[0] = '\0';
    chan->pushback_char = -1;
}

unsigned char *file_get_record_buffer(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return NULL;
    int idx = channel - 1;
    return ctx->channels[idx].record_buffer;
}

void file_close_all(FileContext *ctx) {
    if (!ctx) return;
    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        file_close(ctx, i + 1);
    }
}

bool file_is_open(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return false;
    int idx = channel - 1;
    return (ctx->channels[idx].fp != NULL || ctx->channels[idx].vdev != NULL);
}

FILE *file_get_handle(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return NULL;
    return ctx->channels[channel - 1].fp;
}

BppFileMode file_get_mode(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return FILE_MODE_INPUT;
    return ctx->channels[channel - 1].mode;
}

long file_lof(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return 0;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        FILE *fp = chan->fp;
        long current = ftell(fp);
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, current, SEEK_SET);
        return size;
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_seek) {
            long current = d->dev_seek(d, 0, SEEK_CUR);
            long size = d->dev_seek(d, 0, SEEK_END);
            d->dev_seek(d, current, SEEK_SET);
            return size;
        }
    }
    return 0;
}

long file_loc(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return 0;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        FILE *fp = chan->fp;
        long byte_offset = ftell(fp);
        BppFileMode mode = chan->mode;
        if (mode == FILE_MODE_RANDOM) {
            int rlen = (chan->record_len > 0) ? chan->record_len : 128;
            return (byte_offset / rlen) + 1;
        }
        return byte_offset;
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_seek) {
            return d->dev_seek(d, 0, SEEK_CUR);
        }
    }
    return 0;
}

bool file_eof(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return true;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->pushback_char != -1) {
        return false;
    }

    if (chan->fp) {
        FILE *fp = chan->fp;
        int c = fgetc(fp);
        if (c == EOF) return true;
        ungetc(c, fp);
        return false;
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_poll && d->dev_poll(d) == -1) return true;
        if (d->dev_status && d->dev_status(d) < 0) return true;
        return false; /* Streams don't have natural EOF */
    }
    return true;
}

void file_seek(FileContext *ctx, int channel, long position) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    long byte_pos = 0;
    BppFileMode mode = chan->mode;
    if (mode == FILE_MODE_RANDOM) {
        int rlen = chan->record_len;
        byte_pos = (position - 1) * rlen;
    } else {
        byte_pos = position - 1;
    }
    if (byte_pos < 0) byte_pos = 0;

    if (chan->fp) {
        fseek(chan->fp, byte_pos, SEEK_SET);
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_seek) {
            d->dev_seek(d, byte_pos, SEEK_SET);
        }
    }
}

int file_get_record_len(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return 0;
    return ctx->channels[channel - 1].record_len;
}

VDev *file_get_vdev(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return NULL;
    return ctx->channels[channel - 1].vdev;
}

int file_getc(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->pushback_char != -1) {
        int c = chan->pushback_char;
        chan->pushback_char = -1;
        return c;
    }

    if (chan->fp) {
        return fgetc(chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.getc) {
            return dev->ops.getc(dev);
        }
    }
    return -1;
}

int file_ungetc(FileContext *ctx, int channel, int c) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    ctx->channels[idx].pushback_char = c;
    return c;
}

char *file_gets(FileContext *ctx, int channel, char *buf, size_t size) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !buf || size == 0) return NULL;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        return fgets(buf, (int)size, chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.gets) {
            return dev->ops.gets(dev, buf, size);
        }
        /* Fallback: read character-by-character */
        size_t len = 0;
        int c;
        while (len < size - 1 && (c = file_getc(ctx, channel)) != -1) {
            buf[len++] = (char)c;
            if (c == '\n') break;
        }
        if (len == 0) return NULL;
        buf[len] = '\0';
        return buf;
    }
    return NULL;
}

int file_puts(FileContext *ctx, int channel, const char *s) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !s) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        return fputs(s, chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.puts) {
            return dev->ops.puts(dev, s);
        }
        if (dev->ops.putc) {
            int count = 0;
            while (*s) {
                if (dev->ops.putc(dev, *s) != -1) {
                    count++;
                }
                s++;
            }
            return count;
        }
    }
    return -1;
}

int file_printf(FileContext *ctx, int channel, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    int res = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (res < 0) return -1;
    return file_puts(ctx, channel, buf);
}

int file_putc(FileContext *ctx, int channel, int c) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        return fputc(c, chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.putc) {
            return dev->ops.putc(dev, c);
        }
    }
    return -1;
}

int file_flush(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        return fflush(chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.flush) {
            return dev->ops.flush(dev);
        }
    }
    return 0;
}

BppError file_lock_range(FileContext *ctx, int channel, long start, long end) {
    BppError err; memset(&err, 0, sizeof(err));
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    if (!chan->fp && !chan->vdev) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }

    /* Check for overlap against existing locks */
    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && strcmp(ctx->locks[i].filename, chan->filename) == 0) {
            /* If it's a different channel OR we don't allow same-channel overlaps, fail */
            if (ctx->locks[i].channel != channel) {
                if (!(end < ctx->locks[i].start || start > ctx->locks[i].end)) {
                    err.code = 70; err.message = "Permission denied (lock overlap)";
                    return err;
                }
            }
        }
    }

    /* Find empty slot */
    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (!ctx->locks[i].active) {
            ctx->locks[i].active = true;
            ctx->locks[i].channel = channel;
            strncpy(ctx->locks[i].filename, chan->filename, sizeof(ctx->locks[i].filename)-1);
            ctx->locks[i].filename[sizeof(ctx->locks[i].filename)-1] = '\0';
            ctx->locks[i].start = start;
            ctx->locks[i].end = end;
            return err;
        }
    }

    err.code = 7; err.message = "Out of memory (lock table full)";
    return err;
}

BppError file_unlock_range(FileContext *ctx, int channel, long start, long end) {
    BppError err; memset(&err, 0, sizeof(err));
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    
    bool found = false;
    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && ctx->locks[i].channel == channel && 
            ctx->locks[i].start == start && ctx->locks[i].end == end) {
            ctx->locks[i].active = false;
            found = true;
            break;
        }
    }

    if (!found) {
        err.code = 5; err.message = "Illegal function call (lock not found)";
    }
    return err;
}

BppError file_check_overlap(FileContext *ctx, int channel, long start, long end) {
    BppError err; memset(&err, 0, sizeof(err));
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return err;

    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    if (!chan->fp && !chan->vdev) return err;

    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && ctx->locks[i].channel != channel) {
            if (strcmp(ctx->locks[i].filename, chan->filename) == 0) {
                if (!(end < ctx->locks[i].start || start > ctx->locks[i].end)) {
                    err.code = 70; err.message = "Permission denied (lock overlap)";
                    return err;
                }
            }
        }
    }
    return err;
}

int file_read(FileContext *ctx, int channel, void *buf, int len) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !buf || len <= 0) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    int bytes_read = 0;
    unsigned char *ptr = (unsigned char *)buf;

    if (chan->pushback_char != -1) {
        *ptr++ = (unsigned char)chan->pushback_char;
        chan->pushback_char = -1;
        bytes_read++;
        len--;
    }

    if (len <= 0) return bytes_read;

    if (chan->fp) {
        int read_res = (int)fread(ptr, 1, len, chan->fp);
        if (read_res > 0) {
            bytes_read += read_res;
        }
        return bytes_read > 0 ? bytes_read : -1;
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->dev_read) {
            int read_res = dev->dev_read(dev, ptr, len);
            if (read_res > 0) {
                bytes_read += read_res;
            }
            return bytes_read > 0 ? bytes_read : -1;
        }
        int count = 0;
        int c;
        while (count < len && (c = file_getc(ctx, channel)) != -1) {
            ptr[count++] = (unsigned char)c;
        }
        bytes_read += count;
        return bytes_read > 0 ? bytes_read : -1;
    }
    return -1;
}

int file_write(FileContext *ctx, int channel, const void *buf, int len) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !buf || len <= 0) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];

    if (chan->fp) {
        return (int)fwrite(buf, 1, len, chan->fp);
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->dev_write) {
            return dev->dev_write(dev, buf, len);
        }
        int count = 0;
        const unsigned char *ptr = (const unsigned char *)buf;
        while (count < len) {
            if (file_putc(ctx, channel, ptr[count]) == -1) break;
            count++;
        }
        return count;
    }
    return -1;
}

int file_txn_status(FileContext *ctx) {
    return ctx ? ctx->txn_mode : 0;
}

void file_txn_begin(FileContext *ctx, int mode, bool use_file) {
    if (!ctx) return;
    file_txn_commit(ctx); /* commit any pending first */
    ctx->txn_mode = mode;
    ctx->txn_entry_count = 0;
    ctx->txn_use_file = use_file;
    if (use_file) {
        snprintf(ctx->txn_journal_file, sizeof(ctx->txn_journal_file), "txn_journal.tmp");
        FILE *f = fopen(ctx->txn_journal_file, "wb");
        if (f) fclose(f);
    } else {
        ctx->txn_journal_file[0] = '\0';
    }
}

void file_txn_log_write(FileContext *ctx, int channel, long position, const void *old_data, int len) {
    if (!ctx || ctx->txn_mode == 0 || len <= 0) return;
    if (ctx->txn_entry_count >= 64) return;

    int idx = ctx->txn_entry_count;
    ctx->txn_journal[idx].channel = channel;
    ctx->txn_journal[idx].position = position;
    ctx->txn_journal[idx].len = len;

    if (ctx->txn_use_file) {
        ctx->txn_journal[idx].data = NULL;
        FILE *f = fopen(ctx->txn_journal_file, "ab");
        if (f) {
            int32_t c = channel;
            int32_t pos = (int32_t)position;
            int32_t l = len;
            fwrite(&c, 4, 1, f);
            fwrite(&pos, 4, 1, f);
            fwrite(&l, 4, 1, f);
            fwrite(old_data, 1, len, f);
            fclose(f);
        }
    } else {
        ctx->txn_journal[idx].data = calloc(1, len);
        if (ctx->txn_journal[idx].data) {
            memcpy(ctx->txn_journal[idx].data, old_data, len);
        }
    }
    ctx->txn_entry_count++;
}

BppError file_txn_commit(FileContext *ctx) {
    BppError err; memset(&err, 0, sizeof(err));
    if (!ctx || ctx->txn_mode == 0) return err;

    for (int i = 0; i < ctx->txn_entry_count; ++i) {
        if (ctx->txn_journal[i].data) {
            free(ctx->txn_journal[i].data);
            ctx->txn_journal[i].data = NULL;
        }
    }
    if (ctx->txn_use_file && ctx->txn_journal_file[0]) {
        remove(ctx->txn_journal_file);
        ctx->txn_journal_file[0] = '\0';
    }
    ctx->txn_mode = 0;
    ctx->txn_entry_count = 0;
    return err;
}

BppError file_txn_rollback(FileContext *ctx) {
    BppError err; memset(&err, 0, sizeof(err));
    if (!ctx || ctx->txn_mode == 0) return err;

    if (ctx->txn_use_file) {
        FILE *f = fopen(ctx->txn_journal_file, "rb");
        if (f) {
            typedef struct {
                int32_t channel;
                int32_t position;
                int32_t len;
                unsigned char *data;
            } TempRec;
            TempRec records[64];
            int count = 0;

            while (count < ctx->txn_entry_count && count < 64) {
                int32_t c = 0, pos = 0, l = 0;
                if (fread(&c, 4, 1, f) != 1) break;
                if (fread(&pos, 4, 1, f) != 1) break;
                if (fread(&l, 4, 1, f) != 1) break;
                records[count].channel = c;
                records[count].position = pos;
                records[count].len = l;
                records[count].data = calloc(1, l);
                if (records[count].data) {
                    size_t read_bytes = fread(records[count].data, 1, l, f);
                    (void)read_bytes;
                }
                count++;
            }
            fclose(f);

            for (int i = count - 1; i >= 0; --i) {
                int ch = records[i].channel;
                if (file_is_open(ctx, ch)) {
                    FILE *fp = file_get_handle(ctx, ch);
                    if (fp) {
                        fseek(fp, records[i].position, SEEK_SET);
                        if (records[i].data) {
                            fwrite(records[i].data, 1, records[i].len, fp);
                        }
                        fflush(fp);
                    }
                }
                if (records[i].data) free(records[i].data);
            }
        }
        remove(ctx->txn_journal_file);
        ctx->txn_journal_file[0] = '\0';
    } else {
        for (int i = ctx->txn_entry_count - 1; i >= 0; --i) {
            int ch = ctx->txn_journal[i].channel;
            if (file_is_open(ctx, ch)) {
                FILE *fp = file_get_handle(ctx, ch);
                if (fp) {
                    fseek(fp, ctx->txn_journal[i].position, SEEK_SET);
                    if (ctx->txn_journal[i].data) {
                        fwrite(ctx->txn_journal[i].data, 1, ctx->txn_journal[i].len, fp);
                    }
                    fflush(fp);
                }
            }
        }
    }

    for (int i = 0; i < ctx->txn_entry_count; ++i) {
        if (ctx->txn_journal[i].data) {
            free(ctx->txn_journal[i].data);
            ctx->txn_journal[i].data = NULL;
        }
    }
    ctx->txn_mode = 0;
    ctx->txn_entry_count = 0;
    return err;
}

int file_txn_entry_count(FileContext *ctx) {
    return ctx ? ctx->txn_entry_count : 0;
}


