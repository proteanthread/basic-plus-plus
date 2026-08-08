/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file vdev.h
 * @brief Virtual Device System (VDS) API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares abstract I/O interfaces (Virtual Devices) and management contexts
 *   for routing input and output.
 * - Why it exists: Decouples the compiler, VM, and statement parsers from raw operating system calls
 *   (like printf, scanf, file handles), allowing identical execution behaviors on DOS, console, or SDL frontends.
 * - Why it works this way: It maintains a device registry table. Subsystems perform operations (puts, getc, flush)
 *   by looking up a device by name (e.g. "CON:") and calling its registered function pointers.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional operations inside VDevOps (e.g. seek, stat, ioctl), registry bounds.
 * - What cannot be changed: Obligation to route all console reads and writes through this subsystem.
 * - What to expect: vdev_printf format calls are intercepted and routed to the "CON:" puts operator.
 * - What to do if something breaks: If output goes missing or crashes, verify that the default console device
 *   is registered and active during boot Phase 3.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard character sets are ASCII. Formatting uses standard vsnprintf.
 * - Portability concerns: vsnprintf is C17 standard.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add file streams (FILE:), timers (TIMER:), or graphics windows (GFX:).
 * - How to write external extensions: External hardware plugins (e.g. GPIO/SPI) register custom VDev tables.
 */

#ifndef DEVICE_VDEV_H
#define DEVICE_VDEV_H

#include <stddef.h>
#include <stdbool.h>
#include "memory/memory.h"
#include "types/types.h"

typedef struct VDev VDev;

/* VDevClass taxonomy (Phase 16) */
typedef enum {
    VDCLASS_UNKNOWN = 0,
    VDCLASS_CONSOLE,
    VDCLASS_FILE,
    VDCLASS_SERIAL,
    VDCLASS_PRINTER,
    VDCLASS_AUDIO,
    VDCLASS_NETWORK,
    VDCLASS_GPIO,
    VDCLASS_I2C,
    VDCLASS_SPI,
    VDCLASS_SENSOR,
    VDCLASS_DISPLAY,
    VDCLASS_STORAGE,
    VDCLASS_HID,
    VDCLASS_CAMERA,
    VDCLASS_BRIDGE,
    VDCLASS_BLUETOOTH,
    VDCLASS_CLIPBOARD,
    VDCLASS_PIPE,
    VDCLASS_TIMER,
    VDCLASS_CUSTOM = 99
} VDevClass;

/* Per-device capability flags (Phase 16) */
#define VDCAP_READ    0x0001
#define VDCAP_WRITE   0x0002
#define VDCAP_BINARY  0x0004
#define VDCAP_SEEK    0x0008
#define VDCAP_ASYNC   0x0010
#define VDCAP_HOTPLUG 0x0020
#define VDCAP_CONTROL 0x0040
#define VDCAP_STATUS  0x0080
#define VDCAP_EVENT   0x0100
#define VDCAP_DUPLEX  0x0200
#define VDCAP_BLOCK   0x0400

#define VDCAP_RW       (VDCAP_READ | VDCAP_WRITE)
#define VDCAP_STREAM   (VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX)
#define VDCAP_FILELIKE (VDCAP_READ | VDCAP_WRITE | VDCAP_BINARY | VDCAP_SEEK)

/* Standard IOCTL2 control command codes (Phase 16) */
#define VDIO_RESET        0
#define VDIO_GET_STATUS   1
#define VDIO_SET_BAUD     2
#define VDIO_SET_PIN      3
#define VDIO_GET_PIN      4
#define VDIO_SET_ADDR     5
#define VDIO_SET_TIMEOUT  6
#define VDIO_GET_ERROR    7
#define VDIO_ENUMERATE    8

#define VDIO_SET_CURSOR   (256 + 0)
#define VDIO_GET_CURSOR   (256 + 1)
#define VDIO_SET_COLOR    (256 + 2)
#define VDIO_GET_SIZE     (256 + 3)
#define VDIO_SET_TITLE    (256 + 4)
#define VDIO_SET_MODE     (256 + 5)
#define VDIO_SCROLL       (256 + 6)
#define VDIO_USER         512

/* Table of functional operations for a virtual device */
typedef struct {
    int   (*putc)(VDev *dev, int c);
    int   (*puts)(VDev *dev, const char *s);
    int   (*getc)(VDev *dev);
    char *(*gets)(VDev *dev, char *buf, size_t size);
    int   (*flush)(VDev *dev);
    int   (*cls)(VDev *dev);
} VDevOps;

/* Concrete Virtual Device Instance */
struct VDev {
    const char *name;   /* Case-insensitive name, e.g. "CON:", "NULL:" */
    VDevOps     ops;    /* Operations table */
    void       *priv;   /* Private instance context pointer */

    /* Phase 16 VDev2 extensions */
    VDevClass   dev_class;
    int         dev_caps;
    const char *dev_version;
    const char *dev_description;
    int         dev_req_caps;   /* CAP_ required flags for gating */

    /* Binary I/O */
    int         (*dev_read)(VDev *d, void *buf, int len);
    int         (*dev_write)(VDev *d, const void *buf, int len);
    long        (*dev_seek)(VDev *d, long offset, int whence);

    /* Control */
    int         (*dev_ioctl)(VDev *d, int cmd, void *arg);

    /* Status */
    int         (*dev_status)(VDev *d);
    int         (*dev_poll)(VDev *d);
    const char *(*dev_info)(VDev *d, const char *key);

    /* Lifecycle hooks */
    int         (*dev_open)(VDev *d, const char *path, int mode);
    int         (*dev_close)(VDev *d);
};

/* Opaque Virtual Device Context Manager */
typedef struct VDevContext VDevContext;

/**
 * @brief Initialize the Virtual Device System context.
 */
VDevContext *vdev_init(MemoryContext *mem);

/**
 * @brief Shutdown the Virtual Device System and unregister all devices.
 */
void vdev_shutdown(VDevContext *ctx);

/**
 * @brief Register a device in the active registry.
 * @return true on success, false if table is full or duplicate name.
 */
bool vdev_register(VDevContext *ctx, VDev dev);

/**
 * @brief Find a registered virtual device by name.
 */
VDev *vdev_get(VDevContext *ctx, const char *name);

/**
 * @brief High-level formatted printing to the virtual console device ("CON:").
 */
int vdev_printf(VDevContext *ctx, const char *fmt, ...);

/**
 * @brief Print a string to the virtual console ("CON:").
 */
int vdev_puts(VDevContext *ctx, const char *s);

/**
 * @brief Write a single character to the virtual console ("CON:").
 */
int vdev_putc(VDevContext *ctx, int c);

/**
 * @brief Read a line of input from the virtual console ("CON:").
 */
char *vdev_gets(VDevContext *ctx, char *buf, size_t size);

/**
 * @brief Play standard system beep (800 Hz, 0.25s) or print bell char fallback.
 */
void vdev_play_beep(VDevContext *ctx);

/**
 * @brief Get the length of remaining notes in background playback queue.
 */
int vdev_music_queue_length(void);

/**
 * @brief Clear all notes in background music playback queue.
 */
void vdev_music_clear(void);
void vdev_play_sound_freq(double freq, double duration_seconds);

/**
 * @brief Poll background events (like SDL window close or keyboard queue).
 */
void vdev_gfx_poll_events(void);
void vdev_gfx_force_flush(void);
void vdev_gfx_enable(bool allowed, bool gui_boot);
struct VMContext;
void vdev_gfx_boot_check(struct VMContext *vm);
int  gfx_get_char_at(int row, int col);
int  gfx_get_attr_at(int row, int col);

/* VDev2 wrappers (Phase 16) */
int         vdev_read(VDev *d, void *buf, int len);
int         vdev_write(VDev *d, const void *buf, int len);
long        vdev_seek(VDev *d, long offset, int whence);
int         vdev_ioctl(VDev *d, int cmd, void *arg);
int         vdev_status(VDev *d);
int         vdev_poll(VDev *d);
const char *vdev_info(VDev *d, const char *key);
const char *vdev_class_name(VDevClass cls);
int         vdev_count(VDevContext *ctx);
VDev       *vdev_get_by_index(VDevContext *ctx, int index);
void        vdev_list_all(VDevContext *ctx);

#endif /* DEVICE_VDEV_H */
