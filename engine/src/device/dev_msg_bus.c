// FILENAME: dev_msg_bus.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (dev_msg_bus.h, msg_broker.h, vdev.h), libcore (runtime/format/runtime_snprintf.h, runtime/string/memops.h, runtime/string/strops.h)
// Virtual Message Bus Stream Device (BUS:topic, BUS1..8:) Driver Implementation.
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include <stdbool.h>
#include "device/dev_msg_bus.h"
#include "device/msg_broker.h"

typedef struct {
    char device_name[MSG_NAME_MAX_LEN];
    char topic[MSG_NAME_MAX_LEN];
    char write_buf[MSG_PAYLOAD_MAX_LEN];
    size_t write_len;
} MsgBusDevState;

static MsgBusDevState s_bus_default_state = {
    .device_name = "BUS:",
    .topic = "GLOBAL",
    .write_buf = {0},
    .write_len = 0
};

static int bus_dev_putc(VDev *d, int c) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    if (c == '\n') {
        if (st->write_len > 0) {
            if (st->write_buf[st->write_len - 1] == '\r') {
                st->write_len--;
            }
            st->write_buf[st->write_len] = '\0';
            msg_broker_publish(st->topic, st->write_buf, st->write_len);
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

static int bus_dev_puts(VDev *d, const char *s) {
    if (!s) return 0;
    size_t slen = runtime_strlen(s);
    for (size_t i = 0; s[i] != '\0'; i++) {
        bus_dev_putc(d, (unsigned char)s[i]);
    }
    return (int)slen;
}

static int bus_dev_getc(VDev *d) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    char line[MSG_PAYLOAD_MAX_LEN];
    if (msg_broker_poll_topic(st->topic, line, sizeof(line))) {
        return (unsigned char)line[0];
    }
    return -1;
}

static char *bus_dev_gets(VDev *d, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    if (msg_broker_poll_topic(st->topic, buf, size)) {
        return buf;
    }
    buf[0] = '\0';
    return NULL;
}

static int bus_dev_flush(VDev *d) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    if (st->write_len > 0) {
        if (st->write_buf[st->write_len - 1] == '\r') {
            st->write_len--;
        }
        st->write_buf[st->write_len] = '\0';
        msg_broker_publish(st->topic, st->write_buf, st->write_len);
        st->write_len = 0;
        st->write_buf[0] = '\0';
    }
    return 0;
}

static int bus_dev_cls(VDev *d) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    msg_broker_clear_topic(st->topic);
    return 0;
}

static int bus_dev_read(VDev *d, void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char line[MSG_PAYLOAD_MAX_LEN];
    if (bus_dev_gets(d, line, sizeof(line))) {
        int slen = (int)runtime_strlen(line);
        int copy_len = (slen < len) ? slen : len;
        runtime_memcpy(buf, line, copy_len);
        return copy_len;
    }
    return 0;
}

static int bus_dev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf || len <= 0) return 0;
    char text[MSG_PAYLOAD_MAX_LEN];
    int copy_len = (len < (int)sizeof(text) - 1) ? len : (int)sizeof(text) - 1;
    runtime_memcpy(text, buf, copy_len);
    text[copy_len] = '\0';
    return bus_dev_puts(d, text);
}

static int bus_dev_ioctl(VDev *d, int cmd, void *arg) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    switch (cmd) {
        case 0: // RESET
            msg_broker_clear_topic(st->topic);
            return 0;
        case 1: // MSG COUNT
            return msg_broker_topic_msg_count(st->topic);
        case 2: // SET TOPIC NAME
            if (arg) {
                runtime_snprintf(st->topic, sizeof(st->topic), "%s", (const char *)arg);
                return 0;
            }
            return -1;
        default:
            return 0;
    }
}

static int bus_dev_poll(VDev *d) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    return (msg_broker_topic_msg_count(st->topic) > 0) ? 1 : 0;
}

static const char *bus_dev_info(VDev *d, const char *prop) {
    MsgBusDevState *st = (d && d->priv) ? (MsgBusDevState *)d->priv : &s_bus_default_state;
    if (!prop) return "BUS";
    if (runtime_strcasecmp(prop, "CLASS") == 0) return "BUS";
    if (runtime_strcasecmp(prop, "DRIVER") == 0) return "dev_msg_bus";
    if (runtime_strcasecmp(prop, "TOPIC") == 0) return st->topic;
    if (runtime_strcasecmp(prop, "DESC") == 0) return "Virtual Message Bus & Topic PubSub Stream";
    return "";
}

VDev dev_msg_bus_create(const char *topic_name) {
    msg_broker_init();
    static MsgBusDevState s_bus_pool[8];
    static size_t s_bus_pool_count = 0;
    MsgBusDevState *st = NULL;
    if (s_bus_pool_count < 8) {
        st = &s_bus_pool[s_bus_pool_count++];
    } else {
        HalContext *hal = hal_get();
        if (hal && hal->mem.alloc) {
            st = (MsgBusDevState *)hal->mem.alloc(sizeof(MsgBusDevState));
        }
    }
    if (st) {
        runtime_memset(st, 0, sizeof(MsgBusDevState));
        runtime_snprintf(st->device_name, sizeof(st->device_name), "%s", (topic_name && *topic_name) ? topic_name : "BUS:");
        runtime_snprintf(st->topic, sizeof(st->topic), "GLOBAL");
        if (topic_name && *topic_name) {
            const char *tn = topic_name;
            if (runtime_strncasecmp(tn, "BUS:", 4) == 0) {
                tn += 4;
            }
            if (*tn) {
                runtime_snprintf(st->topic, sizeof(st->topic), "%s", tn);
            }
        }
    }

    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = st ? st->device_name : "BUS:";
    dev.dev_class = VDCLASS_USER;
    dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX | VDCAP_STATUS | VDCAP_CONTROL;
    dev.dev_version = "6.5.2.0";
    dev.dev_description = "Virtual Message Bus & Topic PubSub Stream";
    dev.priv = st ? st : &s_bus_default_state;

    dev.ops.putc = bus_dev_putc;
    dev.ops.puts = bus_dev_puts;
    dev.ops.getc = bus_dev_getc;
    dev.ops.gets = bus_dev_gets;
    dev.ops.flush = bus_dev_flush;
    dev.ops.cls = bus_dev_cls;

    dev.dev_read = bus_dev_read;
    dev.dev_write = bus_dev_write;
    dev.dev_seek = NULL;
    dev.dev_ioctl = bus_dev_ioctl;
    dev.dev_status = NULL;
    dev.dev_poll = bus_dev_poll;
    dev.dev_info = bus_dev_info;
    dev.dev_open = NULL;
    dev.dev_close = NULL;

    return dev;
}
