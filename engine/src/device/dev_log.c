// FILENAME: dev_log.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c, stmt_log.c
// NEEDS: libkernel (dev_log.h, vdev.h), libcore (logger.h)
// Implementation for Virtual Logging Device (LOG:, LOG1..8:, SYSLOG:).
//
// ---- Includes ----

#include "device/dev_log.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "platform/platform.h"
#include "runtime/math/math.h"

typedef struct {
    char name[32];
    int read_cursor;
} LogDevState;

static LogDevState s_log_dev_state = { .name = "LOG:", .read_cursor = 0 };

static int log_dev_putc(VDev *d, int c) {
    (void)d;
    char s[2] = { (char)c, '\0' };
    log_info("%s", s);
    return 1;
}

static int log_dev_puts(VDev *d, const char *s) {
    (void)d;
    if (!s) return 0;
    // Inspect prefix for severity tag [WARN], [ERROR], [DEBUG], [TRACE]
    if (runtime_strncasecmp(s, "[WARN]", 6) == 0) {
        log_warn("%s", s + 6);
    } else if (runtime_strncasecmp(s, "[ERROR]", 7) == 0) {
        log_error("%s", s + 7);
    } else if (runtime_strncasecmp(s, "[DEBUG]", 7) == 0) {
        log_debug("%s", s + 7);
    } else if (runtime_strncasecmp(s, "[TRACE]", 7) == 0) {
        log_trace("%s", s + 7);
    } else if (runtime_strncasecmp(s, "[INFO]", 6) == 0) {
        log_info("%s", s + 6);
    } else {
        log_info("%s", s);
    }
    return (int)runtime_strlen(s);
}

static int log_dev_getc(VDev *d) {
    (void)d;
    const char *last = logger_ring_get_last();
    return (last && last[0]) ? (unsigned char)last[0] : -1;
}

static char *log_dev_gets(VDev *d, char *buf, size_t size) {
    (void)d;
    if (!buf || size == 0) return NULL;
    const char *last = logger_ring_get_last();
    if (!last || !last[0]) {
        buf[0] = '\0';
        return NULL;
    }
    runtime_snprintf(buf, size, "%s", last);
    return buf;
}

static int log_dev_flush(VDev *d) {
    (void)d;
    return 0;
}

static int log_dev_cls(VDev *d) {
    (void)d;
    logger_ring_clear();
    return 0;
}

static int log_dev_read(VDev *d, void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char line[512];
    if (log_dev_gets(d, line, sizeof(line))) {
        int slen = (int)runtime_strlen(line);
        int copy_len = (slen < len) ? slen : len;
        runtime_memcpy(buf, line, copy_len);
        return copy_len;
    }
    return 0;
}

static int log_dev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char text[512];
    int copy_len = (len < (int)sizeof(text) - 1) ? len : (int)sizeof(text) - 1;
    runtime_memcpy(text, buf, copy_len);
    text[copy_len] = '\0';
    return log_dev_puts(d, text);
}

static int log_dev_ioctl(VDev *d, int cmd, void *arg) {
    (void)d;
    switch (cmd) {
        case 0: // Reset/Clear ring buffer
            logger_ring_clear();
            return 0;
        case 1: // Get ring count
            if (arg) *(int *)arg = logger_ring_count();
            return logger_ring_count();
        case 2: // Set runtime_log level
            if (arg) logger_set_level((BppLogLevel)*(int *)arg);
            return 0;
        case 3: // Get runtime_log level
            return (int)logger_get_level();
        case 13: // Status (ring entry count)
            return logger_ring_count();
        default:
            return 0;
    }
}

static int log_dev_status(VDev *d) {
    (void)d;
    return logger_ring_count();
}

static int log_dev_poll(VDev *d) {
    (void)d;
    return (logger_ring_count() > 0) ? 1 : 0;
}

static const char *log_dev_info(VDev *d, const char *key) {
    (void)d;
    if (!key) return NULL;
    if (runtime_strcasecmp(key, "CLASS") == 0) return "LOGGER";
    if (runtime_strcasecmp(key, "DRIVER") == 0) return "dev_log";
    if (runtime_strcasecmp(key, "VERSION") == 0) return "6.5.2";
    if (runtime_strcasecmp(key, "CAPS") == 0) return "RW,STREAM,DUPLEX,LOG_RING,MULTI_LEVEL";
    return NULL;
}

VDev dev_log_create(const char *name) {
    if (name && name[0]) {
        runtime_snprintf(s_log_dev_state.name, sizeof(s_log_dev_state.name), "%s", name);
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = s_log_dev_state.name;
    dev.dev_class = VDCLASS_USER;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Virtual Logging Device & Telemetry Stream";
    dev.priv = &s_log_dev_state;

    dev.ops.putc = log_dev_putc;
    dev.ops.puts = log_dev_puts;
    dev.ops.getc = log_dev_getc;
    dev.ops.gets = log_dev_gets;
    dev.ops.flush = log_dev_flush;
    dev.ops.cls = log_dev_cls;

    dev.dev_read = log_dev_read;
    dev.dev_write = log_dev_write;
    dev.dev_seek = NULL;
    dev.dev_ioctl = log_dev_ioctl;
    dev.dev_status = log_dev_status;
    dev.dev_poll = log_dev_poll;
    dev.dev_info = log_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
