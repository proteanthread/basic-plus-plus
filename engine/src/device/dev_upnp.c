// FILENAME: dev_upnp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (vdev.c), libserver (vnet_nat.c)
// NEEDS: libkernel (vdev.h, dev_upnp.h), libserver (vnet_nat.h)
// Implements UPnP virtual device driver (UPNP:) with DEVCTL command dispatch.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/format/snprintf.h"

#include "device/dev_upnp.h"
#include "runtime/vnet_nat.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static int upnp_putc(VDev *dev, int c) {
    (void)dev;
    return c;
}

static int upnp_puts(VDev *dev, const char *s) {
    (void)dev;
    (void)s;
    return 0;
}

static int upnp_getc(VDev *dev) {
    (void)dev;
    return -1;
}

static char *upnp_gets(VDev *dev, char *buf, size_t size) {
    (void)dev;
    if (!buf || size == 0) return NULL;
    char ip[64] = "";
    vnet_nat_get_external_ip(ip, sizeof(ip));
    runtime_strncpy(buf, ip, size - 1);
    buf[size - 1] = '\0';
    return buf;
}

static int upnp_flush(VDev *dev) {
    (void)dev;
    return 0;
}

static int upnp_cls(VDev *dev) {
    (void)dev;
    return 0;
}

static int upnp_dev_open(VDev *dev, const char *path, int mode) {
    (void)dev;
    (void)path;
    (void)mode;
    return 0;
}

static int upnp_dev_close(VDev *dev) {
    (void)dev;
    return 0;
}

static int upnp_ioctl(VDev *dev, int cmd, void *arg) {
    (void)dev;
    int *iargs = (int *)arg;
    if (!iargs) return -1;

    switch (cmd) {
        case 300: // FORWARD: iargs[0]=ext_port, iargs[1]=int_port
        {
            int ext_port = iargs[0];
            int int_port = (iargs[1] > 0) ? iargs[1] : ext_port;
            BppNatStatus st = vnet_nat_forward(ext_port, int_port, "TCP", "BASIC++ DEVCTL", 0);
            return (st == NAT_STATUS_OK) ? 0 : -1;
        }
        case 301: // UNFORWARD: iargs[0]=ext_port
        {
            int ext_port = iargs[0];
            BppNatStatus st = vnet_nat_unforward(ext_port, "TCP");
            return (st == NAT_STATUS_OK) ? 0 : -1;
        }
        case 303: // STATUS: iargs[0]=ext_port
        {
            int ext_port = iargs[0];
            char info[64] = "";
            BppNatStatus st = vnet_nat_get_status(ext_port, "TCP", info, sizeof(info));
            return (int)st;
        }
        default:
            return 0;
    }
}

VDev dev_upnp_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
    dev.name = name ? name : "UPNP:";
    dev.dev_class = VDCLASS_NETWORK;
    dev.ops.putc = upnp_putc;
    dev.ops.puts = upnp_puts;
    dev.ops.getc = upnp_getc;
    dev.ops.gets = upnp_gets;
    dev.ops.flush = upnp_flush;
    dev.ops.cls = upnp_cls;
    dev.dev_open = upnp_dev_open;
    dev.dev_close = upnp_dev_close;
    dev.dev_ioctl = upnp_ioctl;
    dev.priv = NULL;
    return dev;
}
