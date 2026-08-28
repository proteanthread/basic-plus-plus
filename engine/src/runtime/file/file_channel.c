// FILENAME: file_channel.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file_internal.h)
// Provides core logic and interface definitions for file_channel within BASIC++.
//
// ---- Includes ----

#include "runtime/file_internal.h"

//
// ---- Channel Lifecycle ----

FileContext *file_init(MemoryContext *mem) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) {
        hal_init_hosted();
        hal = hal_get();
    }
    FileContext *ctx = (FileContext *)(hal && hal->mem.alloc ? hal->mem.alloc(sizeof(FileContext)) : NULL);
    if (!ctx) return NULL;

    runtime_memset(ctx, 0, sizeof(FileContext));
    ctx->mem = mem;
    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        ctx->channels[i].handle = IO_HANDLE_INVALID;
        ctx->channels[i].vdev = NULL;
        ctx->channels[i].filename[0] = '\0';
        ctx->channels[i].pushback_char = -1;
        ctx->channels[i].record_buffer = NULL;
        ctx->channels[i].page_buffer = NULL;
        ctx->channels[i].stream_buf = NULL;
        ctx->channels[i].stream_pos = 0;
        ctx->channels[i].stream_len = 0;
        ctx->channels[i].write_accum_len = 0;
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
    HalContext *hal = hal_get();
    file_close_all(ctx);
    for (int i = 0; i < 64; ++i) {
        if (ctx->txn_journal[i].data) {
            if (hal && hal->mem.free) hal->mem.free(ctx->txn_journal[i].data);
            ctx->txn_journal[i].data = NULL;
        }
    }
    if (ctx->txn_use_file && ctx->txn_journal_file[0]) {
        if (hal && hal->io.file_remove) hal->io.file_remove(ctx->txn_journal_file);
    }
    if (hal && hal->mem.free) hal->mem.free(ctx);
}

BppError file_open(FileContext *ctx, VDevContext *vdev_ctx, int channel, const char *filename,
                   BppFileMode mode, BppFileAccess access, BppFileLockMode lock_mode, int record_len) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    HalContext *hal = hal_get();

    if (channel < 1 || channel > BASIC_MAX_OPEN_FILES) {
        err.code = 52;
        err.message = "Bad file number";
        return err;
    }

    int idx = channel - 1;
    if (ctx->channels[idx].handle != IO_HANDLE_INVALID || ctx->channels[idx].vdev != NULL) {
        err.code = 55;
        err.message = "File already open";
        return err;
    }

    VDev *dev = NULL;
    const char *colon = runtime_strchr(filename, ':');
    if (colon && vdev_ctx) {
        char dev_name[256];
        size_t name_len = (size_t)(colon - filename + 1);
        if (name_len < sizeof(dev_name)) {
            runtime_memcpy(dev_name, filename, name_len);
            dev_name[name_len] = '\0';
            dev = vdev_get(vdev_ctx, dev_name);
        }
    }

    if (!dev && vdev_ctx) {
        if (runtime_strcasecmp(filename, "WORKSTN") == 0 || runtime_strcasecmp(filename, "WS") == 0 ||
            runtime_strcasecmp(filename, "5250") == 0 || runtime_strcasecmp(filename, "WORKSTN:") == 0 ||
            runtime_strcasecmp(filename, "WS:") == 0 || runtime_strcasecmp(filename, "5250:") == 0 ||
            runtime_strcasecmp(filename, "TEK") == 0 || runtime_strcasecmp(filename, "TEK:") == 0 ||
            runtime_strcasecmp(filename, "TEKTRONIX") == 0 || runtime_strcasecmp(filename, "TEKTRONIX:") == 0 ||
            runtime_strcasecmp(filename, "4010") == 0 || runtime_strcasecmp(filename, "4010:") == 0 ||
            runtime_strcasecmp(filename, "4014") == 0 || runtime_strcasecmp(filename, "4014:") == 0) {
            dev = vdev_get(vdev_ctx, "CON:");
            if (!dev) dev = vdev_get(vdev_ctx, "CONS:");
            if (!dev) dev = vdev_get(vdev_ctx, "SCRN:");
        }
    }

    if (dev) {
        if (security_check(SECOP_VDEV, 0) != 0) {
            err.code = 70;
            err.message = "Permission denied: virtual device access restricted";
            return err;
        }

        if (!security_module_allowed(dev->dev_req_caps)) {
            err.code = 70;
            err.message = "Permission denied: device capabilities restricted";
            return err;
        }

        if (dev->dev_open) {
            const char *path_param = colon ? colon + 1 : "";
            int open_res = dev->dev_open(dev, path_param, mode);
            if (open_res < 0) {
                err.code = 57;
                err.message = "Device open failed";
                return err;
            }
        }

        ctx->channels[idx].vdev = dev;
        ctx->channels[idx].handle = IO_HANDLE_INVALID;
        runtime_strncpy(ctx->channels[idx].filename, filename, sizeof(ctx->channels[idx].filename) - 1);
        ctx->channels[idx].filename[sizeof(ctx->channels[idx].filename) - 1] = '\0';
        ctx->channels[idx].mode = mode;
        ctx->channels[idx].access = access;
        ctx->channels[idx].lock_mode = lock_mode;
        ctx->channels[idx].record_len = (record_len > 0) ? record_len : BASIC_DEFAULT_RECORD_LEN;
        ctx->channels[idx].pushback_char = -1;
        if (mode == FILE_MODE_RANDOM) {
            ctx->channels[idx].record_buffer = (unsigned char *)(hal && hal->mem.alloc ? hal->mem.alloc(ctx->channels[idx].record_len) : NULL);
            if (!ctx->channels[idx].record_buffer) {
                err.code = 7; err.message = "Out of memory in file_open";
                return err;
            }
            runtime_memset(ctx->channels[idx].record_buffer, 0, ctx->channels[idx].record_len);
        } else {
            ctx->channels[idx].record_buffer = NULL;
        }

        return err;
    }

    const char *real_path = filename;
    if (runtime_strncasecmp(filename, "PRMFL:", 6) == 0) real_path = filename + 6;
    else if (runtime_strncasecmp(filename, "PERM:", 5) == 0) real_path = filename + 5;
    else if (runtime_strncasecmp(filename, "TAPE:", 5) == 0) real_path = filename + 5;
    else if (runtime_strncasecmp(filename, "DISC:", 5) == 0) real_path = filename + 5;
    else if (runtime_strncasecmp(filename, "DISK:", 5) == 0) real_path = filename + 5;
    else if (runtime_strncasecmp(filename, "FILE:", 5) == 0) real_path = filename + 5;


    if (security_get_level() >= SEC_STANDARD) {
        if (security_check_file_path(real_path, 0) != 0) {
            err.code = 70;
            err.message = "Permission denied: path access restricted";
            return err;
        }
    }

    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        if (i != idx && (ctx->channels[i].handle != IO_HANDLE_INVALID || ctx->channels[i].vdev)) {
            if (runtime_strcmp(ctx->channels[i].filename, filename) == 0) {
                BppFileLockMode existing_lock = ctx->channels[i].lock_mode;
                BppFileAccess existing_access = ctx->channels[i].access;
                bool conflict = false;
                if (existing_lock == FILE_LOCK_READ && (access == FILE_ACCESS_READ || access == FILE_ACCESS_READ_WRITE || access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (existing_lock == FILE_LOCK_WRITE && (access == FILE_ACCESS_WRITE || access == FILE_ACCESS_READ_WRITE || access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (existing_lock == FILE_LOCK_READ_WRITE) conflict = true;

                if (lock_mode == FILE_LOCK_READ && (existing_access == FILE_ACCESS_READ || existing_access == FILE_ACCESS_READ_WRITE || existing_access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (lock_mode == FILE_LOCK_WRITE && (existing_access == FILE_ACCESS_WRITE || existing_access == FILE_ACCESS_READ_WRITE || existing_access == FILE_ACCESS_DEFAULT)) conflict = true;
                if (lock_mode == FILE_LOCK_READ_WRITE) conflict = true;

                if (conflict) {
                    err.code = 70;
                    err.message = "Permission denied (sharing violation)";
                    return err;
                }
            }
        }
    }

    const char *fopen_mode = "r";
    switch (mode) {
        case FILE_MODE_INPUT:  fopen_mode = "rb";  break;
        case FILE_MODE_OUTPUT: fopen_mode = "wb";  break;
        case FILE_MODE_APPEND: fopen_mode = "ab";  break;
        case FILE_MODE_BINARY:
        case FILE_MODE_RANDOM: {
            bool exists = false;
            if (hal && hal->io.file_exists) {
                exists = hal->io.file_exists(real_path);
            } else if (hal && hal->io.file_open) {
                IoHandle test = hal->io.file_open(real_path, "r+b");
                if (test != IO_HANDLE_INVALID) {
                    if (hal->io.file_close) hal->io.file_close(test);
                    exists = true;
                }
            }
            fopen_mode = exists ? "r+b" : "w+b";
            break;
        }
    }

    const char *open_target = real_path;
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

    IoHandle handle = (hal && hal->io.file_open) ? hal->io.file_open(open_target, fopen_mode) : IO_HANDLE_INVALID;
    if (handle == IO_HANDLE_INVALID) {
        err.code = 53;
        err.message = "File not found or access denied";
        return err;
    }

    ctx->channels[idx].handle = handle;
    ctx->channels[idx].vdev = NULL;
    runtime_strncpy(ctx->channels[idx].filename, filename, sizeof(ctx->channels[idx].filename) - 1);
    ctx->channels[idx].filename[sizeof(ctx->channels[idx].filename) - 1] = '\0';
    ctx->channels[idx].mode = mode;
    ctx->channels[idx].access = access;
    ctx->channels[idx].lock_mode = lock_mode;
    ctx->channels[idx].record_len = (record_len > 0) ? record_len : BASIC_DEFAULT_RECORD_LEN;
    ctx->channels[idx].pushback_char = -1;
    if (mode == FILE_MODE_RANDOM) {
        ctx->channels[idx].record_buffer = (unsigned char *)(hal && hal->mem.alloc ? hal->mem.alloc(ctx->channels[idx].record_len) : NULL);
        if (!ctx->channels[idx].record_buffer) {
            if (hal && hal->io.file_close) hal->io.file_close(handle);
            ctx->channels[idx].handle = IO_HANDLE_INVALID;
            err.code = 7; err.message = "Out of memory in file_open";
            return err;
        }
        runtime_memset(ctx->channels[idx].record_buffer, 0, ctx->channels[idx].record_len);
    } else {
        ctx->channels[idx].record_buffer = NULL;
    }

    ctx->channels[idx].page_buffer = NULL;

    return err;
}

void file_close(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->write_accum_len > 0 && chan->handle != IO_HANDLE_INVALID && chan->stream_buf) {
        if (hal && hal->io.file_write) {
            hal->io.file_write(chan->handle, chan->stream_buf, 1, chan->write_accum_len);
        }
        chan->write_accum_len = 0;
    }

    if (chan->stream_buf) {
        if (hal && hal->mem.free) hal->mem.free(chan->stream_buf);
        chan->stream_buf = NULL;
        chan->stream_pos = 0;
        chan->stream_len = 0;
        chan->write_accum_len = 0;
    }

    if (chan->handle != IO_HANDLE_INVALID) {
        if (hal && hal->io.file_close) {
            hal->io.file_close(chan->handle);
        }
        chan->handle = IO_HANDLE_INVALID;
    } else if (chan->vdev) {
        if (chan->vdev->dev_close) {
            chan->vdev->dev_close(chan->vdev);
        }
        chan->vdev = NULL;
    }

    if (chan->record_buffer) {
        if (hal && hal->mem.free) hal->mem.free(chan->record_buffer);
        chan->record_buffer = NULL;
    }

    if (chan->page_buffer) {
        if (hal && hal->mem.free) hal->mem.free(chan->page_buffer);
        chan->page_buffer = NULL;
    }

    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && ctx->locks[i].channel == channel) {
            ctx->locks[i].active = false;
        }
    }

    chan->filename[0] = '\0';
    chan->pushback_char = -1;
}

void file_close_all(FileContext *ctx) {
    if (!ctx) return;
    for (int i = 0; i < BASIC_MAX_OPEN_FILES; ++i) {
        file_close(ctx, i + 1);
    }
}

unsigned char *file_get_record_buffer(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return NULL;
    int idx = channel - 1;
    return ctx->channels[idx].record_buffer;
}

bool file_is_open(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return false;
    int idx = channel - 1;
    return (ctx->channels[idx].handle != IO_HANDLE_INVALID || ctx->channels[idx].vdev != NULL);
}

IoHandle file_get_handle(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return IO_HANDLE_INVALID;
    return ctx->channels[channel - 1].handle;
}

BppFileMode file_get_mode(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return FILE_MODE_INPUT;
    return ctx->channels[channel - 1].mode;
}

const char *file_get_filename(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return "";
    return ctx->channels[channel - 1].filename;
}

int file_get_record_len(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return 0;
    return ctx->channels[channel - 1].record_len;
}

VDev *file_get_vdev(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return NULL;
    return ctx->channels[channel - 1].vdev;
}

