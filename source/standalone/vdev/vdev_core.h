/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vdev_core.h
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
 * File: vdev_core.h
 * =====================================================================
 * Standard C interface for virtual device slots and safe dispatching.
 * Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_VDEV_CORE_H
#define STANDALONE_VDEV_CORE_H

/* Maximum device slots */
#define VDEV_MAX 64

/* Built-in device IDs */
typedef enum VDevId {
    VDEV_CON = 0,
    VDEV_ERR = 1,
    VDEV_FILE = 2,
    VDEV_NULL = 3,
    VDEV_TIMER = 4,
    VDEV_COM1 = 5,
    VDEV_COM2 = 6,
    VDEV_LPT1 = 7,
    VDEV_LPT2 = 8,
    VDEV_USER = 9
} VDevId;

/* Device Classes */
typedef enum VDevClass {
    VDCLASS_UNKNOWN = 0,
    VDCLASS_CONSOLE = 1,
    VDCLASS_FILE = 2,
    VDCLASS_SERIAL = 3,
    VDCLASS_PRINTER = 4,
    VDCLASS_AUDIO = 5,
    VDCLASS_NETWORK = 6,
    VDCLASS_GPIO = 7,
    VDCLASS_I2C = 8,
    VDCLASS_SPI = 9,
    VDCLASS_SENSOR = 10,
    VDCLASS_DISPLAY = 11,
    VDCLASS_STORAGE = 12,
    VDCLASS_HID = 13,
    VDCLASS_CAMERA = 14,
    VDCLASS_BRIDGE = 15,
    VDCLASS_BLUETOOTH = 16,
    VDCLASS_CLIPBOARD = 17,
    VDCLASS_PIPE = 18,
    VDCLASS_TIMER = 19,
    VDCLASS_CUSTOM = 99
} VDevClass;

/* Per-Device Capability Flags */
#define VDCAP_NONE 0x0000u
#define VDCAP_READ 0x0001u
#define VDCAP_WRITE 0x0002u
#define VDCAP_BINARY 0x0004u
#define VDCAP_SEEK 0x0008u
#define VDCAP_ASYNC 0x0010u
#define VDCAP_HOTPLUG 0x0020u
#define VDCAP_CONTROL 0x0040u
#define VDCAP_STATUS 0x0080u
#define VDCAP_EVENT 0x0100u
#define VDCAP_DUPLEX 0x0200u
#define VDCAP_BLOCK 0x0400u

#define VDCAP_RW (VDCAP_READ | VDCAP_WRITE)
#define VDCAP_STREAM (VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX)
#define VDCAP_FILELIKE (VDCAP_RW | VDCAP_BINARY | VDCAP_SEEK)

/* IOCTL2 Command Codes */
#define VDIO_RESET 0
#define VDIO_GET_STATUS 1
#define VDIO_SET_BAUD 2
#define VDIO_SET_PIN 3
#define VDIO_GET_PIN 4
#define VDIO_SET_ADDR 5
#define VDIO_SET_TIMEOUT 6
#define VDIO_GET_ERROR 7
#define VDIO_ENUMERATE 8
#define VDIO_USER 256

typedef struct VDev VDev;

struct VDev {
    const char *name;

    /* Output operations */
    int (*dev_putc)(VDev *d, int ch);
    int (*dev_puts)(VDev *d, const char *s);
    int (*dev_flush)(VDev *d);
    int (*dev_cls)(VDev *d);

    /* Input operations */
    int (*dev_getc)(VDev *d);
    int (*dev_gets)(VDev *d, char *buf, int max);

    /* File operations */
    int (*dev_open)(VDev *d, const char *path, const char *mode);
    int (*dev_close)(VDev *d);

    /* Device-specific state */
    void *user_data;

    /* Metadata */
    VDevClass dev_class;
    unsigned int dev_caps;
    const char *dev_version;
    const char *dev_description;
    unsigned int dev_req_caps;

    /* Binary I/O */
    int (*dev_read)(VDev *d, void *buf, int len);
    int (*dev_write)(VDev *d, const void *buf, int len);
    long (*dev_seek)(VDev *d, long offset, int whence);

    /* Control */
    int (*dev_ioctl)(VDev *d, int cmd, void *arg);

    /* Status & discovery */
    int (*dev_status)(VDev *d);
    int (*dev_poll)(VDev *d);
    const char *(*dev_info)(VDev *d, const char *key);
};

/* Core Management */
void vdev_core_init(void);
VDev *vdev_core_get(int id);
int vdev_core_register(VDev *dev);
int vdev_core_register_at(int slot, VDev *dev);
int vdev_core_find_by_name(const char *name);
int vdev_core_find_by_class(VDevClass dev_class, int start_id);
const char *vdev_core_class_name(VDevClass dev_class);
int vdev_core_get_used_count(void);

/* Safe Dispatch Wrappers */
int vdev_core_putc(VDev *d, int ch);
int vdev_core_puts(VDev *d, const char *s);
int vdev_core_printf(VDev *d, const char *fmt, ...);
int vdev_core_getc(VDev *d);
int vdev_core_gets(VDev *d, char *buf, int max);
int vdev_core_flush(VDev *d);
int vdev_core_cls(VDev *d);
int vdev_core_read(VDev *d, void *buf, int len);
int vdev_core_write(VDev *d, const void *buf, int len);
long vdev_core_seek(VDev *d, long offset, int whence);
int vdev_core_ioctl(VDev *d, int cmd, void *arg);
int vdev_core_status(VDev *d);
int vdev_core_poll(VDev *d);
const char *vdev_core_info(VDev *d, const char *key);

#endif /* STANDALONE_VDEV_CORE_H */
