/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file vdev.c
 * @brief Virtual Device System context and registry table.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the registry table for virtual devices, name-based lookups,
 *   and console redirection helpers (vdev_printf, vdev_puts, vdev_putc).
 * - Why it exists: Prevents the core interpreter from directly calling OS I/O APIs.
 *   This makes routing output to console, graphics framebuffers, or files a simple registration change.
 * - Why it works this way: It maintains a static size array of VDev slots. Lookups normalize
 *   queries to upper case. Console helpers lookup "CON:" and execute its registered operations.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: MAX_VDEVS capacity limit (16 by default).
 * - What cannot be changed: Opaque VDevContext definition and the rule that all console calls must redirect to "CON:".
 * - What to expect: vdev_printf formats strings into a stack buffer before calling the target device puts.
 * - What to do if something breaks: If vdev_printf outputs nothing, check if the "CON:" device has been
 *   successfully registered and that its puts pointer is valid.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: CON: is always registered. vsnprintf is C17 compliant and doesn't overflow.
 * - Portability concerns: vsnprintf output buffers are allocated on the stack (1024 bytes), which is safe for core sizes.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add hotplug and async notification messages to the registry.
 * - How to write external extensions: External platforms register their own keyboard/screen devices
 *   under names like "CON:" to capture output.
 */

#include "device/vdev.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define MAX_VDEVS 64

struct VDevContext {
    MemoryContext *mem;
    VDev           devices[MAX_VDEVS];
    size_t         count;
};

/* Case-insensitive string comparison helper */
static int strcmp_nocase(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = toupper((unsigned char)*s1);
        int c2 = toupper((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

VDevContext *vdev_init(MemoryContext *mem) {
    if (!mem) return NULL;
    VDevContext *ctx = (VDevContext *)calloc(1, sizeof(VDevContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->count = 0;
    memset(ctx->devices, 0, sizeof(ctx->devices));
    return ctx;
}

void vdev_shutdown(VDevContext *ctx) {
    /* Virtual devices might have custom allocs in priv,
     * but registration hooks usually handle cleanups. We free context.
     */
    free(ctx);
}

bool vdev_register(VDevContext *ctx, VDev dev) {
    if (!ctx || ctx->count >= MAX_VDEVS || !dev.name) return false;

    /* Check if already registered */
    if (vdev_get(ctx, dev.name) != NULL) {
        return false;
    }

    ctx->devices[ctx->count++] = dev;
    return true;
}

VDev *vdev_get(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;

    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp_nocase(ctx->devices[i].name, name) == 0) {
            return &ctx->devices[i];
        }
    }
    return NULL;
}

int vdev_printf(VDevContext *ctx, const char *fmt, ...) {
    if (!ctx || !fmt) return -1;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int res = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (res < 0) return -1;

    return vdev_puts(ctx, buf);
}

int vdev_puts(VDevContext *ctx, const char *s) {
    if (!ctx || !s) return -1;

    VDev *con = vdev_get(ctx, "CON:");
    if (con && con->ops.puts) {
        return con->ops.puts(con, s);
    }
    return -1;
}

int vdev_putc(VDevContext *ctx, int c) {
    if (!ctx) return -1;

    VDev *con = vdev_get(ctx, "CON:");
    if (con && con->ops.putc) {
        return con->ops.putc(con, c);
    }
    return -1;
}

int vdev_read(VDev *d, void *buf, int len) {
    if (!d || !buf) return -1;
    if (d->dev_read) {
        return d->dev_read(d, buf, len);
    }
    return -1;
}

int vdev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf) return -1;
    if (d->dev_write) {
        return d->dev_write(d, buf, len);
    }
    return -1;
}

long vdev_seek(VDev *d, long offset, int whence) {
    if (!d) return -1;
    if (d->dev_seek) {
        return d->dev_seek(d, offset, whence);
    }
    return -1;
}

int vdev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d) return -1;
    if (d->dev_ioctl) {
        return d->dev_ioctl(d, cmd, arg);
    }
    return -1;
}

int vdev_status(VDev *d) {
    if (!d) return -1;
    if (d->dev_status) {
        return d->dev_status(d);
    }
    return 0; /* Default ready */
}

int vdev_poll(VDev *d) {
    if (!d) return -1;
    if (d->dev_poll) {
        return d->dev_poll(d);
    }
    return 1; /* Default data ready */
}

const char *vdev_info(VDev *d, const char *key) {
    if (!d || !key) return "";
    if (strcmp_nocase(key, "name") == 0) return d->name;
    if (strcmp_nocase(key, "class") == 0) return vdev_class_name(d->dev_class);
    if (strcmp_nocase(key, "version") == 0) return d->dev_version ? d->dev_version : "1.0";
    if (strcmp_nocase(key, "description") == 0) return d->dev_description ? d->dev_description : "";
    if (d->dev_info) {
        const char *res = d->dev_info(d, key);
        if (res) return res;
    }
    return "";
}

const char *vdev_class_name(VDevClass cls) {
    switch (cls) {
        case VDCLASS_UNKNOWN:   return "Unknown";
        case VDCLASS_CONSOLE:   return "Console";
        case VDCLASS_FILE:      return "File";
        case VDCLASS_SERIAL:    return "Serial";
        case VDCLASS_PRINTER:   return "Printer";
        case VDCLASS_AUDIO:     return "Audio";
        case VDCLASS_NETWORK:   return "Network";
        case VDCLASS_GPIO:      return "GPIO";
        case VDCLASS_I2C:       return "I2C";
        case VDCLASS_SPI:       return "SPI";
        case VDCLASS_SENSOR:    return "Sensor";
        case VDCLASS_DISPLAY:   return "Display";
        case VDCLASS_STORAGE:   return "Storage";
        case VDCLASS_HID:       return "HID";
        case VDCLASS_CAMERA:    return "Camera";
        case VDCLASS_BRIDGE:    return "Bridge";
        case VDCLASS_BLUETOOTH: return "Bluetooth";
        case VDCLASS_CLIPBOARD: return "Clipboard";
        case VDCLASS_PIPE:      return "Pipe";
        case VDCLASS_TIMER:     return "Timer";
        case VDCLASS_CUSTOM:    return "Custom";
        default:                return "Unknown";
    }
}

int vdev_count(VDevContext *ctx) {
    return ctx ? (int)ctx->count : 0;
}

VDev *vdev_get_by_index(VDevContext *ctx, int index) {
    if (!ctx || index < 0 || index >= (int)ctx->count) return NULL;
    return &ctx->devices[index];
}

void vdev_list_all(VDevContext *ctx) {
    if (!ctx) return;
    vdev_printf(ctx, "%-5s %-10s %-12s %-8s %s\n", "Slot", "Name", "Class", "Version", "Description");
    vdev_printf(ctx, "%-5s %-10s %-12s %-8s %s\n", "----", "----", "-----", "-------", "-----------");
    for (size_t i = 0; i < ctx->count; ++i) {
        VDev *d = &ctx->devices[i];
        const char *ver = d->dev_version ? d->dev_version : "1.0";
        const char *desc = d->dev_description ? d->dev_description : "";
        vdev_printf(ctx, "%-5d %-10s %-12s %-8s %s\n",
                    (int)i, d->name, vdev_class_name(d->dev_class), ver, desc);
    }
}

/* Global console and graphics state variables for compatibility statements */
int g_cursor_x = 0;
int g_cursor_y = 0;
int g_fg_color_idx = 15;
int g_bg_color_idx = 0;
int g_mouse_x = 0;
int g_mouse_y = 0;
int g_mouse_btn = 0;

