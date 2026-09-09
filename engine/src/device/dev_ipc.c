// FILENAME: dev_ipc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_ipc.h, msg_broker.h, vdev.h), libcore (runtime/format/runtime_snprintf.h, runtime/string/memops.h, runtime/string/strops.h)
// Virtual IPC Endpoint Stream Device (IPC:endpoint, IPC1..8:) Driver Implementation.
//
// ---- Includes ----

#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include <stdbool.h>
#include "device/dev_ipc.h"
#include "device/msg_broker.h"
#include "runtime/format/snprintf.h"

typedef struct {
    char device_name[MSG_NAME_MAX_LEN];
    char endpoint[MSG_NAME_MAX_LEN];
    char write_buf[MSG_PAYLOAD_MAX_LEN];
    size_t write_len;
} MsgIpcDevState;

static MsgIpcDevState s_ipc_default_state = {
    .device_name = "IPC:",
    .endpoint = "DEFAULT",
    .write_buf = {0},
    .write_len = 0
};

static int ipc_dev_putc(VDev *d, int c) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    if (c == '\n') {
        if (st->write_len > 0) {
            if (st->write_buf[st->write_len - 1] == '\r') {
                st->write_len--;
            }
            st->write_buf[st->write_len] = '\0';
            msg_broker_send_ipc(st->endpoint, st->write_buf, st->write_len);
            st->write_len = 0;
            st->write_buf[0] = '\0';
        }
    } else if (c != '\r') {
        if (st->write_len < sizeof(st->write_buf) - 1) {
            st->write_buf[st->write_len++] = (char)c;
            st->write_buf[st->write_len] = '\0';
        }
    }
    return c;
}

static int ipc_dev_puts(VDev *d, const char *s) {
    if (!s) return 0;
    for (size_t i = 0; s[i] != '\0'; i++) {
        ipc_dev_putc(d, (unsigned char)s[i]);
    }
    return (int)runtime_strlen(s);
}

static int ipc_dev_getc(VDev *d) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    char line[MSG_PAYLOAD_MAX_LEN];
    if (msg_broker_recv_ipc(st->endpoint, line, sizeof(line), 0)) {
        return (unsigned char)line[0];
    }
    return -1;
}

static char *ipc_dev_gets(VDev *d, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    if (msg_broker_recv_ipc(st->endpoint, buf, size, 0)) {
        return buf;
    }
    buf[0] = '\0';
    return NULL;
}

static int ipc_dev_flush(VDev *d) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    if (st->write_len > 0) {
        if (st->write_buf[st->write_len - 1] == '\r') {
            st->write_len--;
        }
        st->write_buf[st->write_len] = '\0';
        msg_broker_send_ipc(st->endpoint, st->write_buf, st->write_len);
        st->write_len = 0;
        st->write_buf[0] = '\0';
    }
    return 0;
}

static int ipc_dev_cls(VDev *d) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    msg_broker_clear_ipc(st->endpoint);
    return 0;
}

static int ipc_dev_read(VDev *d, void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char line[MSG_PAYLOAD_MAX_LEN];
    if (ipc_dev_gets(d, line, sizeof(line))) {
        int slen = (int)runtime_strlen(line);
        int copy_len = (slen < len) ? slen : len;
        runtime_memcpy(buf, line, copy_len);
        return copy_len;
    }
    return 0;
}

static int ipc_dev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char text[MSG_PAYLOAD_MAX_LEN];
    int copy_len = (len < (int)sizeof(text) - 1) ? len : (int)sizeof(text) - 1;
    runtime_memcpy(text, buf, copy_len);
    text[copy_len] = '\0';
    return ipc_dev_puts(d, text);
}

static int ipc_dev_ioctl(VDev *d, int cmd, void *arg) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    switch (cmd) {
        case 0: // RESET
            msg_broker_clear_ipc(st->endpoint);
            return 0;
        case 1: // MSG COUNT
            return msg_broker_ipc_msg_count(st->endpoint);
        case 2: // SET ENDPOINT NAME
            if (arg) {
                runtime_snprintf(st->endpoint, sizeof(st->endpoint), "%s", (const char *)arg);
                return 0;
            }
            return -1;
        default:
            return 0;
    }
}

static int ipc_dev_poll(VDev *d) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    return (msg_broker_ipc_msg_count(st->endpoint) > 0) ? 1 : 0;
}

static const char *ipc_dev_info(VDev *d, const char *prop) {
    MsgIpcDevState *st = (d && d->priv) ? (MsgIpcDevState *)d->priv : &s_ipc_default_state;
    if (!prop) return "IPC";
    if (runtime_strcasecmp(prop, "CLASS") == 0) return "IPC";
    if (runtime_strcasecmp(prop, "DRIVER") == 0) return "dev_ipc";
    if (runtime_strcasecmp(prop, "ENDPOINT") == 0) return st->endpoint;
    if (runtime_strcasecmp(prop, "DESC") == 0) return "Virtual Point-to-Point IPC Endpoint Stream";
    return "";
}

VDev dev_ipc_create(const char *endpoint_name) {
    msg_broker_init();
    static MsgIpcDevState s_ipc_pool[8];
    static size_t s_ipc_pool_count = 0;
    MsgIpcDevState *st = NULL;
    if (s_ipc_pool_count < 8) {
        st = &s_ipc_pool[s_ipc_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            st = (MsgIpcDevState *)hal->mem.alloc(sizeof(MsgIpcDevState));
        }
    }
    if (st) {
        runtime_memset(st, 0, sizeof(MsgIpcDevState));
        runtime_snprintf(st->device_name, sizeof(st->device_name), "%s", (endpoint_name && *endpoint_name) ? endpoint_name : "IPC:");
        runtime_snprintf(st->endpoint, sizeof(st->endpoint), "DEFAULT");
        if (endpoint_name && *endpoint_name) {
            const char *ep = endpoint_name;
            if (runtime_strncasecmp(ep, "IPC:", 4) == 0) {
                ep += 4;
            }
            if (*ep) {
                runtime_snprintf(st->endpoint, sizeof(st->endpoint), "%s", ep);
            }
        }
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = st ? st->device_name : "IPC:";
    dev.dev_class = VDCLASS_USER;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Virtual Point-to-Point IPC Endpoint Stream";
    dev.priv = st ? st : &s_ipc_default_state;

    dev.ops.putc = ipc_dev_putc;
    dev.ops.puts = ipc_dev_puts;
    dev.ops.getc = ipc_dev_getc;
    dev.ops.gets = ipc_dev_gets;
    dev.ops.flush = ipc_dev_flush;
    dev.ops.cls = ipc_dev_cls;

    dev.dev_read = ipc_dev_read;
    dev.dev_write = ipc_dev_write;
    dev.dev_seek = NULL;
    dev.dev_ioctl = ipc_dev_ioctl;
    dev.dev_status = NULL;
    dev.dev_poll = ipc_dev_poll;
    dev.dev_info = ipc_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
