// FILENAME: file_record.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file_internal.h)
// Provides core logic and interface definitions for file_record within BASIC++.
//
// ---- Includes ----

#include "runtime/file_internal.h"

//
// ---- File Positioning & Size ----

long file_lof(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return 0;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        int64_t current = (hal && hal->io.file_tell) ? hal->io.file_tell(chan->handle) : 0;
        if (hal && hal->io.file_seek) {
            hal->io.file_seek(chan->handle, 0, IO_SEEK_END);
        }
        int64_t size = (hal && hal->io.file_tell) ? hal->io.file_tell(chan->handle) : 0;
        if (hal && hal->io.file_seek) {
            hal->io.file_seek(chan->handle, current, IO_SEEK_SET);
        }
        return (long)size;
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
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        int64_t byte_offset = (hal && hal->io.file_tell) ? hal->io.file_tell(chan->handle) : 0;
        BppFileMode mode = chan->mode;
        if (mode == FILE_MODE_RANDOM) {
            int rlen = (chan->record_len > 0) ? chan->record_len : 128;
            return (long)((byte_offset / rlen) + 1);
        }
        return (long)byte_offset;
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
    HalContext *hal = hal_get();

    if (chan->pushback_char != -1) {
        return false;
    }

    if (chan->stream_buf && chan->stream_pos < chan->stream_len) {
        return false;
    }

    if (chan->handle != IO_HANDLE_INVALID) {
        if (hal && hal->io.file_eof && hal->io.file_eof(chan->handle)) return true;
        int c = file_getc(ctx, channel);
        if (c == -1) return true;
        file_ungetc(ctx, channel, c);
        return false;
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_poll && d->dev_poll(d) == -1) return true;
        if (d->dev_status && d->dev_status(d) < 0) return true;
        return false;
    }
    return true;
}

void file_seek(FileContext *ctx, int channel, long position) {
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
    chan->stream_pos = 0;
    chan->stream_len = 0;

    if (chan->handle != IO_HANDLE_INVALID) {
        BppFileMode mode = chan->mode;
        int64_t byte_pos = 0;
        if (mode == FILE_MODE_RANDOM) {
            int rlen = (chan->record_len > 0) ? chan->record_len : 128;
            byte_pos = (int64_t)(position - 1) * rlen;
        } else {
            byte_pos = (int64_t)(position - 1);
        }
        if (byte_pos < 0) byte_pos = 0;
        if (hal && hal->io.file_seek) {
            hal->io.file_seek(chan->handle, byte_pos, IO_SEEK_SET);
        }
    } else if (chan->vdev) {
        VDev *d = chan->vdev;
        if (d->dev_seek) {
            long byte_pos = position - 1;
            if (byte_pos < 0) byte_pos = 0;
            d->dev_seek(d, byte_pos, SEEK_SET);
        }
    }
}

//
// ---- Introspection & Type Detection ----

long file_get_recount(FileContext *ctx) {
    return ctx ? ctx->recount : 0;
}

void file_set_recount(FileContext *ctx, long count) {
    if (ctx) ctx->recount = count;
}

long file_get_status(FileContext *ctx, int channel) {
    if (!ctx || channel < 0 || channel >= BASIC_MAX_OPEN_FILES) return -1;
    if (ctx->channels[channel].handle == IO_HANDLE_INVALID && !ctx->channels[channel].vdev) return 0;
    long st = 0;
    if (file_eof(ctx, channel)) st |= 1;
    return st;
}

int file_get_typ(FileContext *ctx, int channel) {
    if (!ctx || channel < 0 || channel >= BASIC_MAX_OPEN_FILES) return 0;
    if (ctx->channels[channel].handle == IO_HANDLE_INVALID && !ctx->channels[channel].vdev) return 0;
    if (file_eof(ctx, channel)) return 3;
    if (ctx->channels[channel].mode == FILE_MODE_RANDOM || ctx->channels[channel].mode == FILE_MODE_BINARY) {
        return 4;
    }

    int c = file_getc(ctx, channel);
    if (c == -1) return 3;
    file_ungetc(ctx, channel, c);
    if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.') {
        return 1;
    }
    return 2;
}

