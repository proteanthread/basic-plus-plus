/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vdev_core.c
 * Subsystem: Virtual Hardware Device Emulator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Emulates serial, network, printer, and clock streams.
 *
 * 2. WHAT TO EXPECT:
 *    Intercepts file I/O calls routing to virtual slots.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Device list definitions.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    VDev driver definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If I/O blocks indefinitely, verify device address.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE VIRTUAL DEVICE REGISTRY SUBSYSTEM (VDev2)
 * File: vdev_core.c
 * ===================================================================== */

#include "vdev_core.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static VDev device_table[VDEV_MAX];
static int device_used = 0;

void vdev_core_init(void)
{
    memset(device_table, 0, sizeof(device_table));
    device_used = 0;
}

VDev *vdev_core_get(int id)
{
    if (id < 0 || id >= VDEV_MAX) return NULL;
    if (device_table[id].name == NULL) return NULL;
    return &device_table[id];
}

int vdev_core_register(VDev *dev)
{
    int slot;
    if (dev == NULL) return -1;

    /* If it is a built-in device slot, write to it directly */
    for (slot = 0; slot < VDEV_MAX; slot++) {
        if (device_table[slot].name != NULL && strcmp(device_table[slot].name, dev->name) == 0) {
            memcpy(&device_table[slot], dev, sizeof(VDev));
            return slot;
        }
    }

    /* Find next empty slot */
    for (slot = VDEV_USER; slot < VDEV_MAX; slot++) {
        if (device_table[slot].name == NULL) {
            memcpy(&device_table[slot], dev, sizeof(VDev));
            if (slot >= device_used) device_used = slot + 1;
            return slot;
        }
    }
    return -1;
}

int vdev_core_register_at(int slot, VDev *dev)
{
    if (slot < 0 || slot >= VDEV_MAX || dev == NULL) return -1;
    memcpy(&device_table[slot], dev, sizeof(VDev));
    if (slot >= device_used) device_used = slot + 1;
    return slot;
}

int vdev_core_find_by_name(const char *name)
{
    int i;
    if (name == NULL) return -1;

    for (i = 0; i < VDEV_MAX; i++) {
        const char *dn = device_table[i].name;
        int j, match;
        if (dn == NULL) continue;

        /* Case-insensitive compare */
        match = 1;
        for (j = 0; ; j++) {
            char a = name[j];
            char b = dn[j];
            if (a >= 'a' && a <= 'z') a = (char)(a - 32);
            if (b >= 'a' && b <= 'z') b = (char)(b - 32);
            if (a != b) { match = 0; break; }
            if (a == '\0') break;
        }
        if (match) return i;
    }
    return -1;
}

int vdev_core_find_by_class(VDevClass dev_class, int start_id)
{
    int i;
    if (start_id < 0) start_id = 0;
    for (i = start_id; i < VDEV_MAX; i++) {
        if (device_table[i].name != NULL && device_table[i].dev_class == dev_class) {
            return i;
        }
    }
    return -1;
}

const char *vdev_core_class_name(VDevClass dev_class)
{
    switch (dev_class) {
        case VDCLASS_UNKNOWN: return "Unknown";
        case VDCLASS_CONSOLE: return "Console";
        case VDCLASS_FILE: return "File";
        case VDCLASS_SERIAL: return "Serial";
        case VDCLASS_PRINTER: return "Printer";
        case VDCLASS_AUDIO: return "Audio";
        case VDCLASS_NETWORK: return "Network";
        case VDCLASS_GPIO: return "GPIO";
        case VDCLASS_I2C: return "I2C";
        case VDCLASS_SPI: return "SPI";
        case VDCLASS_SENSOR: return "Sensor";
        case VDCLASS_DISPLAY: return "Display";
        case VDCLASS_STORAGE: return "Storage";
        case VDCLASS_HID: return "HID";
        case VDCLASS_CAMERA: return "Camera";
        case VDCLASS_BRIDGE: return "Bridge";
        case VDCLASS_BLUETOOTH: return "Bluetooth";
        case VDCLASS_CLIPBOARD: return "Clipboard";
        case VDCLASS_PIPE: return "Pipe";
        case VDCLASS_TIMER: return "Timer";
        case VDCLASS_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

int vdev_core_get_used_count(void)
{
    int i, count = 0;
    for (i = 0; i < VDEV_MAX; i++) {
        if (device_table[i].name != NULL) {
            count = i + 1;
        }
    }
    return count;
}

/* Safe Dispatch Wrappers */

int vdev_core_putc(VDev *d, int ch)
{
    if (d == NULL || d->dev_putc == NULL) return -1;
    return d->dev_putc(d, ch);
}

int vdev_core_puts(VDev *d, const char *s)
{
    if (d == NULL || d->dev_puts == NULL) return -1;
    return d->dev_puts(d, s);
}

int vdev_core_printf(VDev *d, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;

    if (d == NULL || d->dev_puts == NULL) return -1;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return d->dev_puts(d, buf);
}

int vdev_core_getc(VDev *d)
{
    if (d == NULL || d->dev_getc == NULL) return -1;
    return d->dev_getc(d);
}

int vdev_core_gets(VDev *d, char *buf, int max)
{
    if (d == NULL || d->dev_gets == NULL) return -1;
    return d->dev_gets(d, buf, max);
}

int vdev_core_flush(VDev *d)
{
    if (d == NULL || d->dev_flush == NULL) return -1;
    return d->dev_flush(d);
}

int vdev_core_cls(VDev *d)
{
    if (d == NULL || d->dev_cls == NULL) return -1;
    return d->dev_cls(d);
}

int vdev_core_read(VDev *d, void *buf, int len)
{
    if (d == NULL || d->dev_read == NULL) return -1;
    return d->dev_read(d, buf, len);
}

int vdev_core_write(VDev *d, const void *buf, int len)
{
    if (d == NULL || d->dev_write == NULL) return -1;
    return d->dev_write(d, buf, len);
}

long vdev_core_seek(VDev *d, long offset, int whence)
{
    if (d == NULL || d->dev_seek == NULL) return -1;
    return d->dev_seek(d, offset, whence);
}

int vdev_core_ioctl(VDev *d, int cmd, void *arg)
{
    if (d == NULL || d->dev_ioctl == NULL) return -1;
    return d->dev_ioctl(d, cmd, arg);
}

int vdev_core_status(VDev *d)
{
    if (d == NULL || d->dev_status == NULL) return 0;
    return d->dev_status(d);
}

int vdev_core_poll(VDev *d)
{
    if (d == NULL || d->dev_poll == NULL) return 0;
    return d->dev_poll(d);
}

const char *vdev_core_info(VDev *d, const char *key)
{
    if (d == NULL) return NULL;

    if (key != NULL) {
        if (strcmp(key, "name") == 0) return d->name;
        if (strcmp(key, "class") == 0) return vdev_core_class_name(d->dev_class);
        if (strcmp(key, "version") == 0) return d->dev_version;
        if (strcmp(key, "description") == 0) return d->dev_description;
    }

    if (d->dev_info != NULL) return d->dev_info(d, key);
    return NULL;
}
