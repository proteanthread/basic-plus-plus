// FILENAME: dev_serial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_boot.c), libkernel (vdev.c)
// NEEDS: libkernel (dev_serial.h, vdev.h), libplatform (plat_serial.h)
// Hardware and virtual serial device driver (COM1..COM8, R1..R8, AUX).
//
// ---- Includes ----

#include "device/dev_serial.h"
#include "platform/plat_serial.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"
#include "runtime/conv/num_parse.h"
#include "platform/platform.h"

#define SERIAL_RING_SIZE 4096

typedef struct {
    char name[32];
    int port_num;
    PlatformSerialPort *plat_port;
    PlatformSerialConfig config;
    uint8_t rx_ring[SERIAL_RING_SIZE];
    int rx_head;
    int rx_tail;
    uint8_t tx_ring[SERIAL_RING_SIZE];
    int tx_head;
    int tx_tail;
    bool is_open;
} SerialDevContext;

bool dev_serial_parse_params(const char *params, int *out_baud, char *out_parity, int *out_databits, int *out_stopbits) {
    if (!params) return false;
    int baud = 9600;
    char parity = 'N';
    int databits = 8;
    int stopbits = 1;

    // Skip leading spaces
    while (*params && runtime_isspace((unsigned char)*params)) params++;
    if (!*params) {
        if (out_baud) *out_baud = baud;
        if (out_parity) *out_parity = parity;
        if (out_databits) *out_databits = databits;
        if (out_stopbits) *out_stopbits = stopbits;
        return true;
    }

    // Parse baud
    char *endptr = NULL;
    long val = runtime_strtol(params, &endptr, 10);
    if (val > 0) baud = (int)val;
    if (endptr && *endptr == ',') {
        params = endptr + 1;
        while (*params && runtime_isspace((unsigned char)*params)) params++;
        if (*params) {
            parity = (char)runtime_toupper((unsigned char)*params);
            params++;
            if (*params == ',') params++;
            while (*params && runtime_isspace((unsigned char)*params)) params++;
            if (runtime_isdigit((unsigned char)*params)) {
                databits = *params - '0';
                params++;
                if (*params == ',') params++;
                while (*params && runtime_isspace((unsigned char)*params)) params++;
                if (runtime_isdigit((unsigned char)*params)) {
                    stopbits = *params - '0';
                }
            }
        }
    }

    if (out_baud) *out_baud = baud;
    if (out_parity) *out_parity = parity;
    if (out_databits) *out_databits = databits;
    if (out_stopbits) *out_stopbits = stopbits;
    return true;
}

static int serial_putc(VDev *dev, int c) {
    if (!dev || !dev->priv) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    uint8_t ch = (uint8_t)c;
    if (ctx->plat_port) {
        return platform_serial_write(ctx->plat_port, &ch, 1) == 1 ? c : -1;
    }
    int next = (ctx->tx_head + 1) % SERIAL_RING_SIZE;
    if (next != ctx->tx_tail) {
        ctx->tx_ring[ctx->tx_head] = ch;
        ctx->tx_head = next;
        return c;
    }
    return -1;
}

static int serial_puts(VDev *dev, const char *s) {
    if (!dev || !s) return -1;
    while (*s) {
        if (serial_putc(dev, (unsigned char)*s) < 0) return -1;
        s++;
    }
    return 0;
}

static int serial_getc(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    uint8_t ch = 0;
    if (ctx->plat_port) {
        if (platform_serial_read(ctx->plat_port, &ch, 1) == 1) {
            return (int)ch;
        }
        return -1;
    }
    if (ctx->rx_head != ctx->rx_tail) {
        ch = ctx->rx_ring[ctx->rx_tail];
        ctx->rx_tail = (ctx->rx_tail + 1) % SERIAL_RING_SIZE;
        return (int)ch;
    }
    return -1;
}

static char *serial_gets(VDev *dev, char *buf, size_t size) {
    if (!dev || !buf || size <= 1) return NULL;
    size_t idx = 0;
    while (idx + 1 < size) {
        int c = serial_getc(dev);
        if (c < 0) break;
        buf[idx++] = (char)c;
        if (c == '\n') break;
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}

static int serial_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int serial_cls(VDev *dev) {
    (void)dev;
    return 0;
}

static int serial_dev_read(VDev *dev, void *buf, int len) {
    if (!dev || !dev->priv || !buf || len <= 0) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (ctx->plat_port) {
        return platform_serial_read(ctx->plat_port, buf, len);
    }
    int count = 0;
    uint8_t *dst = (uint8_t *)buf;
    while (count < len && ctx->rx_head != ctx->rx_tail) {
        dst[count++] = ctx->rx_ring[ctx->rx_tail];
        ctx->rx_tail = (ctx->rx_tail + 1) % SERIAL_RING_SIZE;
    }
    return count;
}

static int serial_dev_write(VDev *dev, const void *buf, int len) {
    if (!dev || !dev->priv || !buf || len <= 0) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (ctx->plat_port) {
        return platform_serial_write(ctx->plat_port, buf, len);
    }
    int count = 0;
    const uint8_t *src = (const uint8_t *)buf;
    while (count < len) {
        int next = (ctx->tx_head + 1) % SERIAL_RING_SIZE;
        if (next == ctx->tx_tail) break;
        ctx->tx_ring[ctx->tx_head] = src[count++];
        ctx->tx_head = next;
    }
    return count;
}

static int serial_dev_open(VDev *dev, const char *path, int mode) {
    (void)mode;
    if (!dev || !dev->priv) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (path && *path) {
        dev_serial_parse_params(path, &ctx->config.baud_rate, &ctx->config.parity,
                                &ctx->config.data_bits, &ctx->config.stop_bits);
    }
    if (!ctx->plat_port) {
        ctx->plat_port = platform_serial_open(ctx->port_num, &ctx->config);
    } else {
        platform_serial_set_config(ctx->plat_port, &ctx->config);
    }
    ctx->is_open = true;
    return 0;
}

static int serial_dev_close(VDev *dev) {
    if (!dev || !dev->priv) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (ctx->plat_port) {
        platform_serial_close(ctx->plat_port);
        ctx->plat_port = NULL;
    }
    ctx->is_open = false;
    return 0;
}

static int serial_dev_ioctl(VDev *dev, int cmd, void *arg) {
    if (!dev || !dev->priv) return -1;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (cmd == VDIO_SET_BAUD && arg) {
        ctx->config.baud_rate = *(int *)arg;
        if (ctx->plat_port) platform_serial_set_config(ctx->plat_port, &ctx->config);
        return 0;
    }
    return 0;
}

static int serial_dev_status(VDev *dev) {
    if (!dev || !dev->priv) return 0;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    return ctx->is_open ? 1 : 0;
}

static int serial_dev_poll(VDev *dev) {
    if (!dev || !dev->priv) return 0;
    return dev_serial_loc(dev);
}

int dev_serial_loc(VDev *dev) {
    if (!dev || !dev->priv) return 0;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (ctx->plat_port) {
        return platform_serial_rx_available(ctx->plat_port);
    }
    if (ctx->rx_head >= ctx->rx_tail) return ctx->rx_head - ctx->rx_tail;
    return SERIAL_RING_SIZE - (ctx->rx_tail - ctx->rx_head);
}

int dev_serial_lof(VDev *dev) {
    if (!dev || !dev->priv) return SERIAL_RING_SIZE;
    SerialDevContext *ctx = (SerialDevContext *)dev->priv;
    if (ctx->plat_port) {
        return platform_serial_tx_free(ctx->plat_port);
    }
    return SERIAL_RING_SIZE - dev_serial_loc(dev) - 1;
}

int dev_serial_eof(VDev *dev) {
    return dev_serial_loc(dev) == 0 ? 1 : 0;
}

VDev dev_serial_create(const char *name, int port_num) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "COM1:";
    dev.dev_class = VDCLASS_SERIAL;
    dev.dev_version = "6.5.2";
    dev.dev_description = "Hardware Serial Communications Port (UART/RS-232)";
    dev.dev_caps = VDCAP_STREAM | VDCAP_BINARY | VDCAP_CONTROL | VDCAP_STATUS | VDCAP_EVENT;

    dev.ops.putc = serial_putc;
    dev.ops.puts = serial_puts;
    dev.ops.getc = serial_getc;
    dev.ops.gets = serial_gets;
    dev.ops.flush = serial_flush;
    dev.ops.cls = serial_cls;

    dev.dev_read = serial_dev_read;
    dev.dev_write = serial_dev_write;
    dev.dev_open = serial_dev_open;
    dev.dev_close = serial_dev_close;
    dev.dev_ioctl = serial_dev_ioctl;
    dev.dev_status = serial_dev_status;
    dev.dev_poll = serial_dev_poll;

    static SerialDevContext s_serial_pool[8];
    static size_t s_serial_pool_count = 0;
    SerialDevContext *ctx = NULL;
    if (s_serial_pool_count < 8) {
        ctx = &s_serial_pool[s_serial_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            ctx = (SerialDevContext *)hal->mem.alloc(sizeof(SerialDevContext));
        }
    }
    if (ctx) {
        runtime_memset(ctx, 0, sizeof(SerialDevContext));
        runtime_snprintf(ctx->name, sizeof(ctx->name), "%s", name ? name : "COM1:");
        ctx->port_num = port_num > 0 ? port_num : 1;
        ctx->config.baud_rate = 9600;
        ctx->config.data_bits = 8;
        ctx->config.parity = 'N';
        ctx->config.stop_bits = 1;
    }
    dev.name = ctx ? ctx->name : "COM1:";
    dev.priv = ctx;
    return dev;
}
