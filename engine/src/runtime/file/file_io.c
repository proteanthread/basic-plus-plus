// FILENAME: file_io.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file_internal.h)
// Provides core logic and interface definitions for file_io within BASIC++.
//
// ---- Includes ----

#include "runtime/file_internal.h"

//
// ---- Character & Stream Reading ----

int file_getc(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->pushback_char != -1) {
        int c = chan->pushback_char;
        chan->pushback_char = -1;
        return c;
    }

    if (chan->stream_buf && chan->stream_pos < chan->stream_len) {
        return (unsigned char)chan->stream_buf[chan->stream_pos++];
    }

    if (chan->handle != IO_HANDLE_INVALID) {
        if (!chan->stream_buf) {
            chan->stream_buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(STREAM_CHUNK_SIZE) : NULL);
            chan->stream_pos = 0;
            chan->stream_len = 0;
            chan->write_accum_len = 0;
        }
        if (chan->stream_buf) {
            chan->stream_pos = 0;
            size_t read_bytes = (hal && hal->io.file_read) ? hal->io.file_read(chan->handle, chan->stream_buf, 1, STREAM_CHUNK_SIZE) : 0;
            chan->stream_len = read_bytes;
            if (chan->stream_len > 0) {
                return (unsigned char)chan->stream_buf[chan->stream_pos++];
            }
            return -1;
        }
        unsigned char c = 0;
        if (hal && hal->io.file_read && hal->io.file_read(chan->handle, &c, 1, 1) == 1) {
            return c;
        }
        return -1;
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.getc) {
            return dev->ops.getc(dev);
        }
        if (dev->dev_read) {
            unsigned char c = 0;
            if (dev->dev_read(dev, &c, 1) == 1) {
                return (int)c;
            }
        }
    }
    return -1;
}

int file_ungetc(FileContext *ctx, int channel, int c) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    if (chan->stream_buf && chan->stream_pos > 0) {
        chan->stream_pos--;
        chan->stream_buf[chan->stream_pos] = (char)c;
        return c;
    }
    chan->pushback_char = c;
    return c;
}

char *file_gets(FileContext *ctx, int channel, char *buf, size_t size) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !buf || size == 0) return NULL;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        if (chan->pushback_char != -1) {
            buf[0] = (char)chan->pushback_char;
            chan->pushback_char = -1;
            if (buf[0] == '\n') {
                buf[1] = '\0';
                return buf;
            }
            if (size > 2) {
                if (file_gets(ctx, channel, buf + 1, size - 1)) {
                    return buf;
                }
            }
            buf[1] = '\0';
            return buf;
        }

        if (!chan->stream_buf) {
            chan->stream_buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(STREAM_CHUNK_SIZE) : NULL);
            chan->stream_pos = 0;
            chan->stream_len = 0;
            chan->write_accum_len = 0;
        }

        if (!chan->stream_buf) {
            size_t count = 0;
            while (count < size - 1) {
                int c = file_getc(ctx, channel);
                if (c == -1) break;
                buf[count++] = (char)c;
                if (c == '\n') break;
            }
            if (count == 0) return NULL;
            buf[count] = '\0';
            return buf;
        }

        size_t out_idx = 0;
        while (out_idx < size - 1) {
            if (chan->stream_pos >= chan->stream_len) {
                chan->stream_pos = 0;
                size_t read_bytes = (hal && hal->io.file_read) ? hal->io.file_read(chan->handle, chan->stream_buf, 1, STREAM_CHUNK_SIZE) : 0;
                chan->stream_len = read_bytes;
                if (chan->stream_len == 0) {
                    if (out_idx == 0) return NULL;
                    break;
                }
            }

            char *start = chan->stream_buf + chan->stream_pos;
            size_t remaining = chan->stream_len - chan->stream_pos;
            char *nl = (char *)runtime_memchr(start, '\n', remaining);

            if (nl) {
                size_t copy_len = (size_t)(nl - start) + 1;
                if (out_idx + copy_len >= size) {
                    copy_len = size - 1 - out_idx;
                }
                runtime_memcpy(buf + out_idx, start, copy_len);
                out_idx += copy_len;
                chan->stream_pos += (size_t)(nl - start) + 1;
                break;
            } else {
                size_t copy_len = remaining;
                if (out_idx + copy_len >= size) {
                    copy_len = size - 1 - out_idx;
                }
                runtime_memcpy(buf + out_idx, start, copy_len);
                out_idx += copy_len;
                chan->stream_pos += copy_len;
                if (out_idx >= size - 1) break;
            }
        }
        buf[out_idx] = '\0';
        return buf;
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.gets) {
            return dev->ops.gets(dev, buf, size);
        }
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


//
// ---- Character & Stream Writing ----

int file_puts(FileContext *ctx, int channel, const char *s) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !s) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        size_t slen = runtime_strlen(s);
        if (!chan->stream_buf) {
            chan->stream_buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(STREAM_CHUNK_SIZE) : NULL);
            chan->stream_pos = 0;
            chan->stream_len = 0;
            chan->write_accum_len = 0;
        }
        if (chan->stream_buf) {
            if (chan->write_accum_len + slen < STREAM_CHUNK_SIZE) {
                runtime_memcpy(chan->stream_buf + chan->write_accum_len, s, slen);
                chan->write_accum_len += slen;
                return (int)slen;
            }
            if (chan->write_accum_len > 0) {
                if (hal && hal->io.file_write) {
                    hal->io.file_write(chan->handle, chan->stream_buf, 1, chan->write_accum_len);
                }
                chan->write_accum_len = 0;
            }
            if (slen >= STREAM_CHUNK_SIZE) {
                return (hal && hal->io.file_write) ? (int)hal->io.file_write(chan->handle, s, 1, slen) : -1;
            }
            runtime_memcpy(chan->stream_buf, s, slen);
            chan->write_accum_len = slen;
            return (int)slen;
        }
        return (hal && hal->io.file_write) ? (int)hal->io.file_write(chan->handle, s, 1, slen) : -1;
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
        if (dev->dev_write) {
            return dev->dev_write(dev, s, (int)runtime_strlen(s));
        }
    }
    return -1;
}

int file_write_raw(FileContext *ctx, int channel, const char *data, size_t len) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !data || len == 0) return 0;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        if (!chan->stream_buf) {
            chan->stream_buf = (char *)(hal && hal->mem.alloc ? hal->mem.alloc(STREAM_CHUNK_SIZE) : NULL);
            chan->stream_pos = 0;
            chan->stream_len = 0;
            chan->write_accum_len = 0;
        }
        if (chan->stream_buf) {
            if (chan->write_accum_len + len < STREAM_CHUNK_SIZE) {
                runtime_memcpy(chan->stream_buf + chan->write_accum_len, data, len);
                chan->write_accum_len += len;
                return (int)len;
            }
            if (chan->write_accum_len > 0) {
                if (hal && hal->io.file_write) {
                    hal->io.file_write(chan->handle, chan->stream_buf, 1, chan->write_accum_len);
                }
                chan->write_accum_len = 0;
            }
            if (len >= STREAM_CHUNK_SIZE) {
                return (hal && hal->io.file_write) ? (int)hal->io.file_write(chan->handle, data, 1, len) : -1;
            }
            runtime_memcpy(chan->stream_buf, data, len);
            chan->write_accum_len = len;
            return (int)len;
        }
        return (hal && hal->io.file_write) ? (int)hal->io.file_write(chan->handle, data, 1, len) : -1;
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.puts) {
            return dev->ops.puts(dev, data);
        }
        if (dev->ops.putc) {
            int count = 0;
            for (size_t i = 0; i < len; i++) {
                if (dev->ops.putc(dev, data[i]) != -1) count++;
            }
            return count;
        }
        if (dev->dev_write) {
            return dev->dev_write(dev, data, (int)len);
        }
    }
    return -1;
}

int file_printf(FileContext *ctx, int channel, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    int res = runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (res < 0) return -1;
    return file_puts(ctx, channel, buf);
}

int file_putc(FileContext *ctx, int channel, int c) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    char ch[2] = { (char)c, '\0' };
    return file_puts(ctx, channel, ch);
}

int file_flush(FileContext *ctx, int channel) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    if (chan->write_accum_len > 0 && chan->handle != IO_HANDLE_INVALID && chan->stream_buf) {
        if (hal && hal->io.file_write) {
            hal->io.file_write(chan->handle, chan->stream_buf, 1, chan->write_accum_len);
        }
        chan->write_accum_len = 0;
    }

    if (chan->handle != IO_HANDLE_INVALID) {
        return (hal && hal->io.file_flush) ? hal->io.file_flush(chan->handle) : 0;
    } else if (chan->vdev) {
        VDev *dev = chan->vdev;
        if (dev->ops.flush) {
            return dev->ops.flush(dev);
        }
    }
    return 0;
}

int file_read(FileContext *ctx, int channel, void *buf, int len) {
    if (!ctx || channel < 1 || channel > BASIC_MAX_OPEN_FILES || !buf || len <= 0) return -1;
    int idx = channel - 1;
    BppFileChannel *chan = &ctx->channels[idx];
    HalContext *hal = hal_get();

    int bytes_read = 0;
    unsigned char *ptr = (unsigned char *)buf;

    if (chan->pushback_char != -1) {
        *ptr++ = (unsigned char)chan->pushback_char;
        chan->pushback_char = -1;
        bytes_read++;
        len--;
    }

    if (len <= 0) return bytes_read;

    if (chan->handle != IO_HANDLE_INVALID) {
        int read_res = (hal && hal->io.file_read) ? (int)hal->io.file_read(chan->handle, ptr, 1, len) : -1;
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
    HalContext *hal = hal_get();

    if (chan->handle != IO_HANDLE_INVALID) {
        return (hal && hal->io.file_write) ? (int)hal->io.file_write(chan->handle, buf, 1, len) : -1;
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

