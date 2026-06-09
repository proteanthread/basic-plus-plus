/*
 * =====================================================================
 * BASIC++ Interpreter - vdev.h
 * =====================================================================
 *
 * Virtual Device System interface (VDev2 - Phase 16).
 *
 * PURPOSE:
 *   Provides a portable abstraction layer between the BASIC++ interpreter
 *   core and all host I/O operations. Every byte of input and output
 *   passes through a VDev - the interpreter never calls printf, fgets,
 *   fopen, or any stdio function directly. This isolation guarantees:
 *
 *     1. Deterministic behavior across platforms
 *     2. Clean separation of language semantics from host services
 *     3. Extensibility - custom devices can be registered at runtime
 *     4. Testability - I/O can be intercepted and verified
 *     5. Security - device access can be gated by capability checks
 *
 * VDEV2 EXTENSIONS (Phase 16 - Futureproofing):
 *   The original VDev was character-based (putc/puts/getc/gets) —
 *   designed for 1970s-80s devices (TTYs, printers, serial ports).
 *   VDev2 adds:
 *     - Device classes (VDCLASS_GPIO, VDCLASS_SENSOR, etc.)
 *     - Per-device capability flags (VDCAP_READ, VDCAP_BINARY, etc.)
 *     - Binary I/O (dev_read/dev_write for raw byte buffers)
 *     - Seekable devices (dev_seek)
 *     - Extended control (dev_ioctl with integer command codes)
 *     - Status and polling (dev_status/dev_poll for non-blocking I/O)
 *     - Device info queries (dev_info for key-value metadata)
 *     - Device discovery (find by name, find by class, enumerate)
 *     - 64 device slots (up from 16) for modern device counts
 *
 *   BACKWARD COMPATIBILITY:
 *     All new fields are optional. Existing VDev code that only sets
 *     the original fields (name, dev_putc, dev_puts, etc.) continues
 *     to work unchanged — new fields are zero/NULL by default via
 *     memset in vdev_register() or static initialization.
 *
 * BUILT-IN DEVICES:
 *   VDEV_CON  - Console device (stdout + stdin)
 *   VDEV_ERR  - Error output device (stderr)
 *   VDEV_FILE - File I/O device (fopen/fclose/fprintf/fgets)
 *
 * HOW TO WRITE EXTERNAL DEVICES:
 *   1. Allocate a VDev struct (static or heap).
 *   2. Fill in name, dev_class, dev_caps, and function pointers.
 *   3. Set unsupported operations to NULL.
 *   4. Call vdev_register() to add it to the device table.
 *   5. Access it by ID via vdev_get() or by name via vdev_find_by_name().
 *
 *   Example - a GPIO pin device (Raspberry Pi):
 *
 *     static int gpio_read(VDev *d, void *buf, int len) {
 *         int *val = (int *)buf;
 *         (void)len;
 *         *val = gpio_pin_read((int)(long)d->user_data);
 *         return sizeof(int);
 *     }
 *     static int gpio_write(VDev *d, const void *buf, int len) {
 *         int val = *(const int *)buf;
 *         (void)len;
 *         gpio_pin_write((int)(long)d->user_data, val);
 *         return sizeof(int);
 *     }
 *
 *     VDev gpio_dev;
 *     memset(&gpio_dev, 0, sizeof(gpio_dev));
 *     gpio_dev.name = "GPIO17:";
 *     gpio_dev.dev_class = VDCLASS_GPIO;
 *     gpio_dev.dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_BINARY;
 *     gpio_dev.dev_read = gpio_read;
 *     gpio_dev.dev_write = gpio_write;
 *     gpio_dev.dev_description = "GPIO pin 17";
 *     gpio_dev.user_data = (void *)17;
 *     vdev_register(&gpio_dev);
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_VDEV_H
#define BASICPP_VDEV_H

/* =====================================================================
 * Device Identifiers
 * =====================================================================
 * Built-in device IDs. Custom devices start at VDEV_USER.
 * Phase 16: Expanded from 16 to 64 slots for modern device counts.
 */
typedef enum VDevId {
    VDEV_CON  = 0,    /* Console device - stdout + stdin */
    VDEV_ERR  = 1,    /* Error device - stderr */
    VDEV_FILE = 2,    /* File I/O device */
    VDEV_USER = 3,    /* First user-registerable slot */
    VDEV_MAX  = 64    /* Maximum device slots (was 16) */
} VDevId;

/* =====================================================================
 * Device Classes (Phase 16)
 * =====================================================================
 * Every device belongs to a class that describes its general type.
 * Used for discovery (vdev_find_by_class), documentation, and
 * security scoping. A class is metadata — it does not constrain
 * which function pointers the device implements.
 *
 * WHY THIS EXISTS:
 *   Modern systems have many device types. A program may need to
 *   say "find me a sensor" or "find me a serial port" without
 *   knowing the specific device name. Device classes enable this.
 *
 * FUTUREPROOFING:
 *   New classes can be added without breaking existing code.
 *   Just add a new enum value before VDCLASS_CUSTOM.
 *   VDCLASS_CUSTOM (99) is reserved for user-defined classes.
 */
typedef enum VDevClass {
    VDCLASS_UNKNOWN  = 0,   /* unclassified / legacy device */
    VDCLASS_CONSOLE  = 1,   /* terminal, screen, keyboard */
    VDCLASS_FILE     = 2,   /* file system */
    VDCLASS_SERIAL   = 3,   /* RS-232, UART, USB-serial */
    VDCLASS_PRINTER  = 4,   /* line printer, PDF output */
    VDCLASS_AUDIO    = 5,   /* sound card, speaker, MIDI */
    VDCLASS_NETWORK  = 6,   /* TCP, UDP, HTTP, WebSocket */
    VDCLASS_GPIO     = 7,   /* digital I/O pins (RPi, Arduino) */
    VDCLASS_I2C      = 8,   /* I2C bus (sensors, displays) */
    VDCLASS_SPI      = 9,   /* SPI bus (SD cards, flash) */
    VDCLASS_SENSOR   = 10,  /* accelerometer, GPS, gyro, temp */
    VDCLASS_DISPLAY  = 11,  /* framebuffer, LCD, OLED, GUI */
    VDCLASS_STORAGE  = 12,  /* block device, SD card, USB drive */
    VDCLASS_HID      = 13,  /* touchscreen, joystick, gamepad */
    VDCLASS_CAMERA   = 14,  /* camera, video capture */
    VDCLASS_BRIDGE   = 15,  /* external MCU (Arduino via serial) */
    VDCLASS_BLUETOOTH = 16, /* Bluetooth/BLE devices */
    VDCLASS_CLIPBOARD = 17, /* system clipboard */
    VDCLASS_PIPE     = 18,  /* process pipe / IPC */
    VDCLASS_TIMER    = 19,  /* hardware/software timer */
    VDCLASS_CUSTOM   = 99   /* user-defined class */
} VDevClass;

/* =====================================================================
 * Per-Device Capability Flags (Phase 16)
 * =====================================================================
 * Each registered device declares its capabilities via a bitfield.
 * These flags describe WHAT the device CAN DO, independent of
 * security permissions (which determine what it MAY do).
 *
 * The interpreter and BASIC programs can query these flags via
 * vdev_info() or DEVICECAP$() to adapt behavior.
 */
#define VDCAP_NONE     0x0000u  /* no capabilities */
#define VDCAP_READ     0x0001u  /* device supports reading */
#define VDCAP_WRITE    0x0002u  /* device supports writing */
#define VDCAP_BINARY   0x0004u  /* device supports binary I/O */
#define VDCAP_SEEK     0x0008u  /* device supports seeking */
#define VDCAP_ASYNC    0x0010u  /* device supports non-blocking */
#define VDCAP_HOTPLUG  0x0020u  /* device can appear/disappear */
#define VDCAP_CONTROL  0x0040u  /* device supports IOCTL2 */
#define VDCAP_STATUS   0x0080u  /* device has queryable status */
#define VDCAP_EVENT    0x0100u  /* device can generate events */
#define VDCAP_DUPLEX   0x0200u  /* device supports full-duplex */
#define VDCAP_BLOCK    0x0400u  /* device does block I/O */

/* Convenience macros */
#define VDCAP_RW       (VDCAP_READ | VDCAP_WRITE)
#define VDCAP_STREAM   (VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX)
#define VDCAP_FILELIKE (VDCAP_RW | VDCAP_BINARY | VDCAP_SEEK)

/* =====================================================================
 * IOCTL2 Command Codes (Phase 16)
 * =====================================================================
 * Standardized command codes for dev_ioctl(). Devices may define
 * their own commands starting at VDIO_USER.
 */
#define VDIO_RESET       0   /* reset device to default state */
#define VDIO_GET_STATUS  1   /* get device status word */
#define VDIO_SET_BAUD    2   /* set baud rate (serial) */
#define VDIO_SET_PIN     3   /* set pin mode (GPIO) */
#define VDIO_GET_PIN     4   /* get pin value (GPIO) */
#define VDIO_SET_ADDR    5   /* set I2C/SPI address */
#define VDIO_SET_TIMEOUT 6   /* set I/O timeout (ms) */
#define VDIO_GET_ERROR   7   /* get last error code */
#define VDIO_ENUMERATE   8   /* list sub-devices */
#define VDIO_USER        256 /* first user-defined command */

/* Forward declaration */
typedef struct VDev VDev;

/* =====================================================================
 * Virtual Device Structure (VDev2)
 * =====================================================================
 *
 * Each device is a table of function pointers plus metadata.
 * Operations that the device does not support should be set to NULL.
 * The helper functions in vdev.c return error codes for NULL ops.
 *
 * BACKWARD COMPATIBILITY:
 *   The first 10 fields (name through user_data) are identical to
 *   the original VDev struct. Code that only sets these fields
 *   works unchanged — the new fields default to 0/NULL.
 *
 * NEW FIELDS (Phase 16):
 *   dev_class       - Device classification for discovery
 *   dev_caps        - What the device can do (VDCAP_ flags)
 *   dev_version     - Version string (informational)
 *   dev_description - Human-readable description
 *   dev_req_caps    - Required module CAP_ flags for security
 *   dev_read        - Binary read (raw bytes)
 *   dev_write       - Binary write (raw bytes)
 *   dev_seek        - Seek to position (for seekable devices)
 *   dev_ioctl       - Extended control commands
 *   dev_status      - Device health/readiness check
 *   dev_poll        - Non-blocking data availability check
 *   dev_info        - Key-value metadata queries
 */
struct VDev {
    /* === Original fields (Phase 3 — DO NOT REORDER) === */
    const char *name;

    /* Output operations */
    int  (*dev_putc)(VDev *d, int ch);
    int  (*dev_puts)(VDev *d, const char *s);
    int  (*dev_flush)(VDev *d);
    int  (*dev_cls)(VDev *d);

    /* Input operations */
    int  (*dev_getc)(VDev *d);
    int  (*dev_gets)(VDev *d, char *buf, int max);

    /* File operations */
    int  (*dev_open)(VDev *d, const char *path, const char *mode);
    int  (*dev_close)(VDev *d);

    /* Device-specific state (e.g., FILE*, terminal handle, etc.) */
    void *user_data;

    /* === Phase 16: VDev2 Extensions === */

    /* Metadata */
    VDevClass    dev_class;       /* device classification */
    unsigned int dev_caps;        /* VDCAP_ capability bitfield */
    const char  *dev_version;     /* version string, or NULL */
    const char  *dev_description; /* human-readable description */
    unsigned int dev_req_caps;    /* required module CAP_ flags */

    /* Binary I/O (for sensors, cameras, raw data, block devices) */
    int  (*dev_read)(VDev *d, void *buf, int len);
    int  (*dev_write)(VDev *d, const void *buf, int len);
    long (*dev_seek)(VDev *d, long offset, int whence);

    /* Extended control (replaces single-string IOCTL) */
    int  (*dev_ioctl)(VDev *d, int cmd, void *arg);

    /* Status and discovery */
    int  (*dev_status)(VDev *d);
    int  (*dev_poll)(VDev *d);
    const char *(*dev_info)(VDev *d, const char *key);
};

/* =====================================================================
 * Device System Functions
 * =====================================================================
 */

/*
 * vdev_init - Initialize the virtual device system.
 *
 * Creates and registers the three built-in devices (CON:, ERR:, FILE:).
 * Must be called once during interpreter boot, before any I/O.
 */
void vdev_init(void);

/*
 * vdev_get - Get a device by ID.
 *
 * Returns a pointer to the registered VDev, or NULL if the slot
 * is empty. Built-in devices (VDEV_CON, VDEV_ERR, VDEV_FILE)
 * are always available after vdev_init().
 */
VDev *vdev_get(int id);

/*
 * vdev_register - Register a custom device.
 *
 * Copies the VDev struct into the next available slot (VDEV_USER+).
 * Returns the assigned device ID, or -1 if the table is full.
 */
int vdev_register(VDev *dev);

/* =====================================================================
 * Device Discovery (Phase 16)
 * =====================================================================
 * These functions allow BASIC programs and modules to find devices
 * by name or class without knowing their slot IDs.
 */

/*
 * vdev_find_by_name - Find a device by name (case-insensitive).
 *
 * Returns the device slot ID, or -1 if not found.
 * Matches against the full device name (e.g., "COM1:").
 */
int vdev_find_by_name(const char *name);

/*
 * vdev_find_by_class - Find the next device of a given class.
 *
 * Searches starting from slot 'start_from'. Returns the slot ID
 * of the first matching device, or -1 if none found.
 * Use start_from=0 for the first search, then start_from=id+1
 * to find subsequent devices of the same class.
 */
int vdev_find_by_class(VDevClass cls, int start_from);

/*
 * vdev_count_class - Count devices of a given class.
 */
int vdev_count_class(VDevClass cls);

/*
 * vdev_list_all - Print all registered devices.
 *
 * Shows slot ID, name, class, capabilities, and description.
 * Used by the DEVICES command.
 */
void vdev_list_all(void);

/*
 * vdev_class_name - Return human-readable class name.
 */
const char *vdev_class_name(VDevClass cls);

/* =====================================================================
 * Convenience I/O Functions (Original)
 * =====================================================================
 * These wrap common patterns for cleaner calling code.
 * They handle NULL device pointers and NULL function pointers safely.
 */

int vdev_putc(VDev *d, int ch);
int vdev_puts(VDev *d, const char *s);
int vdev_printf(VDev *d, const char *fmt, ...);
int vdev_getc(VDev *d);
int vdev_gets(VDev *d, char *buf, int max);
int vdev_flush(VDev *d);
int vdev_cls(VDev *d);

/* =====================================================================
 * Binary I/O Convenience (Phase 16)
 * =====================================================================
 * These wrap the new dev_read/dev_write/dev_seek function pointers.
 * NULL-safe: return -1 if the device or function pointer is NULL.
 */

/*
 * vdev_read - Read binary data from a device.
 * Returns bytes actually read, or -1 on error/unsupported.
 */
int vdev_read(VDev *d, void *buf, int len);

/*
 * vdev_write - Write binary data to a device.
 * Returns bytes actually written, or -1 on error/unsupported.
 */
int vdev_write(VDev *d, const void *buf, int len);

/*
 * vdev_seek - Seek to a position on a device.
 * whence: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END
 * Returns the new position, or -1 on error/unsupported.
 */
long vdev_seek(VDev *d, long offset, int whence);

/* =====================================================================
 * Control & Status Convenience (Phase 16)
 * =====================================================================
 */

/*
 * vdev_ioctl - Send a control command to a device.
 * cmd: command code (VDIO_* or device-specific)
 * arg: command-specific argument (may be NULL)
 * Returns 0 on success, -1 on error/unsupported.
 */
int vdev_ioctl(VDev *d, int cmd, void *arg);

/*
 * vdev_status - Query device status.
 * Returns: 0 = ready, 1 = busy, -1 = error/offline
 */
int vdev_status(VDev *d);

/*
 * vdev_poll - Non-blocking check for data availability.
 * Returns: 1 = data ready, 0 = no data, -1 = error
 */
int vdev_poll(VDev *d);

/*
 * vdev_info - Query device metadata by key.
 * Keys: "class", "caps", "version", "description", "status"
 * Returns a static string, or NULL if key unknown.
 */
const char *vdev_info(VDev *d, const char *key);

/* =====================================================================
 * Platform Convenience Functions (Original)
 * =====================================================================
 */

void vdev_beep(void);
void vdev_sound(int freq_hz, int duration_ms);
void vdev_sleep(int duration_ms);
int  vdev_inkey(void);

#endif /* BASICPP_VDEV_H */
