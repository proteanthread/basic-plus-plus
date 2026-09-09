// FILENAME: vdev_remote.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (nil_transport.h, nil_transport.c, string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (vdev_remote.h)
// Implements REMOTE: virtual device driver.
//
// ---- Includes ----

#include "device/vdev_remote.h"
#include "runtime/nil_transport.h"

#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static int remote_vdev_read(VDev *d, void *buf, int len) {
    (void)d;
    if (!buf || len <= 0) return 0;
    const char *sim_resp = "OK\n";
    size_t slen = runtime_strlen(sim_resp);
    size_t cpy = (size_t)len < slen ? (size_t)len : slen;
    runtime_memcpy(buf, sim_resp, cpy);
    return (int)cpy;
}

static int remote_vdev_write(VDev *d, const void *buf, int len) {
    (void)d;
    if (!buf || len <= 0) return 0;
    return len;
}

static int remote_vdev_close(VDev *d) {
    (void)d;
    return 0;
}

VDev vdev_remote_create(const char *name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(VDev));

    dev.name = name ? name : "REMOTE:";
    dev.dev_class = VDCLASS_NETWORK;
    dev.dev_caps = VDCAP_RW | VDCAP_STREAM;
    dev.dev_version = "1.0";
    dev.dev_description = "RFC 51 NIL Universal Remote Transport Device";

    dev.dev_read = remote_vdev_read;
    dev.dev_write = remote_vdev_write;
    dev.dev_close = remote_vdev_close;

    return dev;
}
