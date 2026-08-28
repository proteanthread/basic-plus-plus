// FILENAME: file_txn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file_internal.h)
// Provides core logic and interface definitions for file_txn within BASIC++.
//
// ---- Includes ----

#include "runtime/file_internal.h"

//
// ---- Range Locking ----

BppError file_lock_range(FileContext *ctx, int channel, long start, long end) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    if (chan->handle == IO_HANDLE_INVALID && !chan->vdev) {
        err.code = 52; err.message = "Bad file number";
        return err;
    }

    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && runtime_strcmp(ctx->locks[i].filename, chan->filename) == 0) {
            if (ctx->locks[i].channel != channel) {
                if (!(end < ctx->locks[i].start || start > ctx->locks[i].end)) {
                    err.code = 70; err.message = "Permission denied (lock overlap)";
                    return err;
                }
            }
        }
    }

    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (!ctx->locks[i].active) {
            ctx->locks[i].active = true;
            ctx->locks[i].channel = channel;
            runtime_strncpy(ctx->locks[i].filename, chan->filename, sizeof(ctx->locks[i].filename) - 1);
            ctx->locks[i].filename[sizeof(ctx->locks[i].filename) - 1] = '\0';
            ctx->locks[i].start = start;
            ctx->locks[i].end = end;
            return err;
        }
    }

    err.code = 7; err.message = "Out of memory (lock table full)";
    return err;
}

BppError file_unlock_range(FileContext *ctx, int channel, long start, long end) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return err;

    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    if (chan->handle == IO_HANDLE_INVALID && !chan->vdev) return err;

    for (int i = 0; i < BASIC_MAX_RANGE_LOCKS; ++i) {
        if (ctx->locks[i].active && ctx->locks[i].channel != channel) {
            if (runtime_strcmp(ctx->locks[i].filename, chan->filename) == 0) {
                if (!(end < ctx->locks[i].start || start > ctx->locks[i].end)) {
                    err.code = 70; err.message = "Permission denied (lock overlap)";
                    return err;
                }
            }
        }
    }
    return err;
}

//
// ---- Transaction Management ----

int file_txn_status(FileContext *ctx) {
    return ctx ? ctx->txn_mode : 0;
}

void file_txn_begin(FileContext *ctx, int mode, bool use_file) {
    if (!ctx) return;
    file_txn_commit(ctx);
    ctx->txn_mode = mode;
    ctx->txn_entry_count = 0;
    ctx->txn_use_file = use_file;
    if (use_file) {
        HalContext *hal = hal_get();
        runtime_snprintf(ctx->txn_journal_file, sizeof(ctx->txn_journal_file), "txn_journal.tmp");
        if (hal && hal->io.file_open) {
            IoHandle f = hal->io.file_open(ctx->txn_journal_file, "wb");
            if (f != IO_HANDLE_INVALID && hal->io.file_close) {
                hal->io.file_close(f);
            }
        }
    } else {
        ctx->txn_journal_file[0] = '\0';
    }
}

void file_txn_log_write(FileContext *ctx, int channel, long position, const void *old_data, int len) {
    if (!ctx || ctx->txn_mode == 0 || len <= 0) return;
    if (ctx->txn_entry_count >= 64) return;
    HalContext *hal = hal_get();

    int idx = ctx->txn_entry_count;
    ctx->txn_journal[idx].channel = channel;
    ctx->txn_journal[idx].position = position;
    ctx->txn_journal[idx].len = len;

    if (ctx->txn_use_file) {
        ctx->txn_journal[idx].data = NULL;
        if (hal && hal->io.file_open) {
            IoHandle f = hal->io.file_open(ctx->txn_journal_file, "ab");
            if (f != IO_HANDLE_INVALID) {
                int32_t c = channel;
                int32_t pos = (int32_t)position;
                int32_t l = len;
                if (hal->io.file_write) {
                    hal->io.file_write(f, &c, 4, 1);
                    hal->io.file_write(f, &pos, 4, 1);
                    hal->io.file_write(f, &l, 4, 1);
                    hal->io.file_write(f, old_data, 1, len);
                }
                if (hal->io.file_close) hal->io.file_close(f);
            }
        }
    } else {
        ctx->txn_journal[idx].data = (unsigned char *)(hal && hal->mem.alloc ? hal->mem.alloc(len) : NULL);
        if (ctx->txn_journal[idx].data) {
            runtime_memcpy(ctx->txn_journal[idx].data, old_data, len);
        }
    }
    ctx->txn_entry_count++;
}

BppError file_txn_commit(FileContext *ctx) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!ctx || ctx->txn_mode == 0) return err;
    HalContext *hal = hal_get();

    for (int i = 0; i < ctx->txn_entry_count; ++i) {
        if (ctx->txn_journal[i].data) {
            if (hal && hal->mem.free) hal->mem.free(ctx->txn_journal[i].data);
            ctx->txn_journal[i].data = NULL;
        }
    }
    if (ctx->txn_use_file && ctx->txn_journal_file[0]) {
        if (hal && hal->io.file_remove) hal->io.file_remove(ctx->txn_journal_file);
        ctx->txn_journal_file[0] = '\0';
    }
    ctx->txn_mode = 0;
    ctx->txn_entry_count = 0;
    return err;
}

BppError file_txn_rollback(FileContext *ctx) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!ctx || ctx->txn_mode == 0) return err;
    HalContext *hal = hal_get();

    if (ctx->txn_use_file) {
        if (hal && hal->io.file_open) {
            IoHandle f = hal->io.file_open(ctx->txn_journal_file, "rb");
            if (f != IO_HANDLE_INVALID) {
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
                    if (hal->io.file_read(f, &c, 4, 1) != 1) break;
                    if (hal->io.file_read(f, &pos, 4, 1) != 1) break;
                    if (hal->io.file_read(f, &l, 4, 1) != 1) break;
                    records[count].channel = c;
                    records[count].position = pos;
                    records[count].len = l;
                    records[count].data = (unsigned char *)(hal->mem.alloc ? hal->mem.alloc(l) : NULL);
                    if (records[count].data) {
                        hal->io.file_read(f, records[count].data, 1, l);
                    }
                    count++;
                }
                if (hal->io.file_close) hal->io.file_close(f);

                for (int i = count - 1; i >= 0; --i) {
                    int ch = records[i].channel;
                    if (file_is_open(ctx, ch)) {
                        IoHandle h = file_get_handle(ctx, ch);
                        if (h != IO_HANDLE_INVALID && hal->io.file_seek && hal->io.file_write) {
                            hal->io.file_seek(h, records[i].position, IO_SEEK_SET);
                            if (records[i].data) {
                                hal->io.file_write(h, records[i].data, 1, records[i].len);
                            }
                            if (hal->io.file_flush) hal->io.file_flush(h);
                        }
                    }
                    if (records[i].data && hal->mem.free) hal->mem.free(records[i].data);
                }
            }
        }
        if (hal && hal->io.file_remove) hal->io.file_remove(ctx->txn_journal_file);
        ctx->txn_journal_file[0] = '\0';
    } else {
        for (int i = ctx->txn_entry_count - 1; i >= 0; --i) {
            int ch = ctx->txn_journal[i].channel;
            if (file_is_open(ctx, ch)) {
                IoHandle h = file_get_handle(ctx, ch);
                if (h != IO_HANDLE_INVALID && hal && hal->io.file_seek && hal->io.file_write) {
                    hal->io.file_seek(h, ctx->txn_journal[i].position, IO_SEEK_SET);
                    if (ctx->txn_journal[i].data) {
                        hal->io.file_write(h, ctx->txn_journal[i].data, 1, ctx->txn_journal[i].len);
                    }
                    if (hal->io.file_flush) hal->io.file_flush(h);
                }
            }
        }
    }

    for (int i = 0; i < ctx->txn_entry_count; ++i) {
        if (ctx->txn_journal[i].data) {
            if (hal && hal->mem.free) hal->mem.free(ctx->txn_journal[i].data);
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

