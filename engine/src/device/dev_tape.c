// FILENAME: dev_tape.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_tape.h, vdev.h), libplatform (plat_fs.h)
// Implementation for Tape & Cassette Audio Synthesizer Bridge (CAS1..8:, TAP1..8:, C:, MAG1..8:).
//
// ---- Includes ----

#include "device/dev_tape.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"

#define TAPE_BUF_SIZE (64 * 1024)

typedef struct {
    char name[32];
    char file_path[256];
    bool file_open;
    bool is_wav;
    bool motor_on;
    int  baud_rate;
    TapeModulation modulation;
    uint8_t buffer[TAPE_BUF_SIZE];
    size_t head;
    size_t tail;
    size_t count;
    void *file_handle;
} TapeContext;

static TapeContext s_tape_instances[10];
static bool s_tape_inited = false;

static void tape_init_all(void) {
    if (s_tape_inited) return;
    runtime_memset(s_tape_instances, 0, sizeof(s_tape_instances));
    for (int i = 0; i < 10; i++) {
        runtime_snprintf(s_tape_instances[i].name, sizeof(s_tape_instances[i].name), "CAS%d:", i);
        s_tape_instances[i].baud_rate = 1500;
        s_tape_instances[i].modulation = TAPE_MOD_TRS80_1500;
        s_tape_instances[i].motor_on = false;
    }
    s_tape_inited = true;
}

static void write_wav_header(void *fp, uint32_t raw_pcm_bytes) {
    if (!fp) return;
    uint32_t sample_rate = 22050;
    uint32_t byte_rate = sample_rate; // 8-bit mono
    uint16_t block_align = 1;
    uint16_t bits_per_sample = 8;
    uint32_t riff_chunk_size = 36 + raw_pcm_bytes;

    platform_file_seek(fp, 0, 0); // SEEK_SET
    platform_file_write(fp, "RIFF", 4);
    platform_file_write(fp, &riff_chunk_size, 4);
    platform_file_write(fp, "WAVEfmt ", 8);

    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels = 1; // Mono
    platform_file_write(fp, &subchunk1_size, 4);
    platform_file_write(fp, &audio_format, 2);
    platform_file_write(fp, &num_channels, 2);
    platform_file_write(fp, &sample_rate, 4);
    platform_file_write(fp, &byte_rate, 4);
    platform_file_write(fp, &block_align, 2);
    platform_file_write(fp, &bits_per_sample, 2);

    platform_file_write(fp, "data", 4);
    platform_file_write(fp, &raw_pcm_bytes, 4);
}

static int tape_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    TapeContext *ctx = (TapeContext *)dev->priv;
    if (ctx->count >= TAPE_BUF_SIZE) return -1;
    ctx->buffer[ctx->head] = (uint8_t)c;
    ctx->head = (ctx->head + 1) % TAPE_BUF_SIZE;
    ctx->count++;
    return c;
}

static int tape_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    int written = 0;
    while (*s) {
        if (tape_putc(dev, (int)(unsigned char)*s) == -1) break;
        s++;
        written++;
    }
    return written;
}

static int tape_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    TapeContext *ctx = (TapeContext *)dev->priv;
    if (ctx->count == 0) return -1;
    int c = ctx->buffer[ctx->tail];
    ctx->tail = (ctx->tail + 1) % TAPE_BUF_SIZE;
    ctx->count--;
    return c;
}

static char *tape_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size == 0) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int ch = tape_getc(dev);
        if (ch == -1) break;
        buf[idx++] = (char)ch;
        if (ch == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int tape_flush(VDev *dev) {
    if (!dev || !dev->priv) return 0;
    TapeContext *ctx = (TapeContext *)dev->priv;
    if (ctx->file_handle && ctx->count > 0) {
        if (ctx->is_wav) {
            // Synthesize simple square wave FSK audio PCM for recorded bytes
            // Mark=2400Hz, Space=1200Hz at 22050Hz sample rate
            for (size_t i = 0; i < ctx->count; i++) {
                uint8_t byte = ctx->buffer[(ctx->tail + i) % TAPE_BUF_SIZE];
                for (int b = 0; b < 8; b++) {
                    int bit = (byte >> b) & 1;
                    int cycles = bit ? 2 : 1;
                    int samples_per_half = bit ? 5 : 9;
                    for (int c = 0; c < cycles; c++) {
                        for (int s = 0; s < samples_per_half; s++) {
                            uint8_t high = 0xF0;
                            platform_file_write(ctx->file_handle, &high, 1);
                        }
                        for (int s = 0; s < samples_per_half; s++) {
                            uint8_t low = 0x10;
                            platform_file_write(ctx->file_handle, &low, 1);
                        }
                    }
                }
            }
        } else {
            // Raw binary stream write
            for (size_t i = 0; i < ctx->count; i++) {
                uint8_t b = ctx->buffer[(ctx->tail + i) % TAPE_BUF_SIZE];
                platform_file_write(ctx->file_handle, &b, 1);
            }
        }
        ctx->head = 0;
        ctx->tail = 0;
        ctx->count = 0;
    }
    return 0;
}

static int tape_cls(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    TapeContext *ctx = (TapeContext *)dev->priv;
    ctx->head = 0;
    ctx->tail = 0;
    ctx->count = 0;
    return 0;
}

static int tape_dev_open(VDev *d, const char *path, int mode) {
    if (!d || !d->priv) return -1;
    TapeContext *ctx = (TapeContext *)d->priv;
    if (path && path[0]) {
        runtime_snprintf(ctx->file_path, sizeof(ctx->file_path), "%s", path);
        size_t plen = runtime_strlen(path);
        ctx->is_wav = (plen >= 4 && runtime_strcasecmp(path + plen - 4, ".wav") == 0);
        const char *fmode = (mode == 1) ? "rb" : "wb";
        ctx->file_handle = platform_file_open(path, fmode);
        if (ctx->file_handle) {
            ctx->file_open = true;
            ctx->motor_on = true;
            if (ctx->is_wav && mode != 1) {
                // Reserve 44 bytes for WAV header
                uint8_t zero_hdr[44] = {0};
                platform_file_write(ctx->file_handle, zero_hdr, 44);
            }
            return 0;
        }
    }
    ctx->file_open = false;
    ctx->motor_on = true;
    return 0;
}

static int tape_dev_close(VDev *d) {
    if (!d || !d->priv) return -1;
    TapeContext *ctx = (TapeContext *)d->priv;
    if (ctx->file_handle) {
        tape_flush(d);
        if (ctx->is_wav) {
            int64_t total_len = platform_file_tell(ctx->file_handle);
            if (total_len > 44) {
                write_wav_header(ctx->file_handle, (uint32_t)(total_len - 44));
            }
        }
        platform_file_close(ctx->file_handle);
        ctx->file_handle = NULL;
    }
    ctx->file_open = false;
    ctx->motor_on = false;
    return 0;
}

static int tape_dev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d || !d->priv) return -1;
    TapeContext *ctx = (TapeContext *)d->priv;
    switch (cmd) {
        case 43: // MOTOR_ON
            ctx->motor_on = true;
            return 0;
        case 44: // MOTOR_OFF
            ctx->motor_on = false;
            return 0;
        case 45: // REWIND
            ctx->head = 0;
            ctx->tail = 0;
            ctx->count = 0;
            if (ctx->file_handle) {
                platform_file_seek(ctx->file_handle, ctx->is_wav ? 44 : 0, 0); // SEEK_SET
            }
            return 0;
        case 2: // SET_BAUD
            if (arg) ctx->baud_rate = *(int *)arg;
            return 0;
        case 13: // STATUS
            return ctx->motor_on ? 1 : 0;
        default:
            return 0;
    }
}

static int tape_dev_status(VDev *d) {
    if (!d || !d->priv) return 0;
    TapeContext *ctx = (TapeContext *)d->priv;
    return ctx->motor_on ? 1 : 0;
}

static int tape_dev_poll(VDev *d) {
    if (!d || !d->priv) return 0;
    TapeContext *ctx = (TapeContext *)d->priv;
    return (ctx->count > 0) ? 1 : 0;
}

static const char *tape_dev_info(VDev *d, const char *key) {
    if (!d || !key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "TAPE";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_tape";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "RW,STREAM,FSK,WAV_SYNTH,MOTOR_CTRL";
    return NULL;
}

VDev dev_tape_create(const char *name) {
    tape_init_all();
    int ch = 1;
    if (name && name[0]) {
        for (const char *p = name; *p; p++) {
            if (*p >= '0' && *p <= '9') { ch = *p - '0'; break; }
        }
    }
    if (ch < 0 || ch > 9) ch = 1;
    TapeContext *ctx = &s_tape_instances[ch];
    if (name && name[0]) {
        runtime_snprintf(ctx->name, sizeof(ctx->name), "%s", name);
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = ctx->name;
    dev.dev_class = VDCLASS_TAPE;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_STATUS | VDCAP_CONTROL | VDCAP_BINARY;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Cassette & Tape Audio Synthesizer Bridge";
    dev.priv = ctx;

    dev.ops.putc = tape_putc;
    dev.ops.puts = tape_puts;
    dev.ops.getc = tape_getc;
    dev.ops.gets = tape_gets;
    dev.ops.flush = tape_flush;
    dev.ops.cls = tape_cls;

    dev.dev_read = NULL;
    dev.dev_write = NULL;
    dev.dev_seek = NULL;
    dev.dev_ioctl = tape_dev_ioctl;
    dev.dev_status = tape_dev_status;
    dev.dev_poll = tape_dev_poll;
    dev.dev_info = tape_dev_info;
    dev.dev_open = tape_dev_open;
    dev.dev_close = tape_dev_close;

    return dev;
}
