/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vterm.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Virtual Terminal (VTERM:) VHAL Device Driver. Tracks dimensions,
 *    cursor offsets, and formatting wraps.
 * ===================================================================== */

#include "vdev.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int cursor_x;
    int cursor_y;
    int width;
    int height;
} VTermState;

static VTermState g_vterm_state = { 0, 0, 80, 25 };

static int vterm_putc(VDev *d, int ch) {
    (void)d;
    putchar(ch);
    if (ch == '\n') {
        g_vterm_state.cursor_x = 0;
        g_vterm_state.cursor_y++;
    } else if (ch == '\r') {
        g_vterm_state.cursor_x = 0;
    } else {
        g_vterm_state.cursor_x++;
        if (g_vterm_state.cursor_x >= g_vterm_state.width) {
            g_vterm_state.cursor_x = 0;
            g_vterm_state.cursor_y++;
            putchar('\n');
        }
    }
    return 0;
}

static int vterm_puts(VDev *d, const char *s) {
    while (*s) {
        vterm_putc(d, *s++);
    }
    return 0;
}

static int vterm_ioctl(VDev *d, int cmd, void *arg) {
    (void)d;
    if (cmd == 1) { // Set Width
        if (arg) g_vterm_state.width = *(int *)arg;
        return 0;
    }
    if (cmd == 2) { // Set Height
        if (arg) g_vterm_state.height = *(int *)arg;
        return 0;
    }
    if (cmd == 3) { // Get Cursor Pos
        if (arg) {
            int *coords = (int *)arg;
            coords[0] = g_vterm_state.cursor_x;
            coords[1] = g_vterm_state.cursor_y;
        }
        return 0;
    }
    return -1;
}

void vterm_init_device(VDev *d) {
    d->name = "VTERM:";
    d->dev_putc = vterm_putc;
    d->dev_puts = vterm_puts;
    d->dev_flush = NULL;
    d->dev_cls = NULL;
    d->dev_getc = NULL;
    d->dev_gets = NULL;
    d->dev_open = NULL;
    d->dev_close = NULL;
    d->dev_ioctl = vterm_ioctl;
    d->user_data = &g_vterm_state;
    d->dev_class = VDCLASS_CONSOLE;
    d->dev_caps = VDCAP_WRITE;
    d->dev_version = "1.0";
    d->dev_description = "Virtual Terminal (width/height tracking)";
    d->dev_req_caps = 0;
}
