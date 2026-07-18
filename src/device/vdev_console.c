/**
 * @file vdev_console.c
 * @brief Console ("CON:") virtual device implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the puts, putc, getc, gets, and flush operations mapping to actual
 *   host console streams (stdout, stdin).
 * - Why it exists: Provides the actual bridge between the Virtual Device System abstraction and the host OS
 *   standard input/output terminals.
 * - Why it works this way: It connects virtual callbacks directly to standard C stdout/stdin streams (or platform-specific keys),
 *   fulfilling the console I/O requirements for the REPL, PRINT, and INPUT statements.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Mapping to custom keyboard scan engines or color attribute terminals.
 * - What cannot be changed: Callback structures (must conform to standard puts/getc signatures).
 * - What to expect: Writing to this device outputs characters immediately onto the user terminal.
 * - What to do if something breaks: If input/output blocks or fails, verify standard stream availability
 *   on the target host system.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Host standard streams (stdout, stdin) are initialized and writable.
 * - Portability concerns: ANSI escape sequences used for color or clearing screen require terminal support.
 *   Uses portable C17 <stdio.h> wrappers.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add screen cursor coordinates, colors, and line editing hooks.
 * - How to write external extensions: Custom console platforms (e.g. SDL graphics screen) register their own
 *   puts/getc implementations using the same "CON:" identifier to redirect output.
 */

#include "bpp_vdev.h"
#include "bpp_vcon.h"
#include "bpp_logger.h"
#include <stdio.h>
#include <string.h>

static int con_putc(VDev *dev, int c) {
    (void)dev;
    extern VConContext *g_vcon_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        vcon_write_char(g_vcon_context, active_idx, c);
    }
    char c_str[2] = {(char)c, '\0'};
    bpp_log_write_out(c_str, 1);
    int res = putchar(c);
    return res;
}

static int con_puts(VDev *dev, const char *s) {
    (void)dev;
    extern VConContext *g_vcon_context;
    if (g_vcon_context && s) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        const char *p = s;
        while (*p) {
            vcon_write_char(g_vcon_context, active_idx, (unsigned char)*p++);
        }
    }
    if (s) {
        bpp_log_write_out(s, strlen(s));
    }
    int res = fputs(s, stdout);
    return res;
}

static int con_getc(VDev *dev) {
    (void)dev;
    return getchar();
}

static char *con_gets(VDev *dev, char *buf, size_t size) {
    (void)dev;
    if (!buf || size == 0) return NULL;
    char *res = fgets(buf, (int)size, stdin);
    return res;
}

static int con_flush(VDev *dev) {
    (void)dev;
    return fflush(stdout);
}

static int con_cls(VDev *dev) {
    (void)dev;
    extern VConContext *g_vcon_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        vcon_clear(g_vcon_context, active_idx);
    }
    /* ANSI escape to clear screen and reset cursor */
    return fputs("\033[2J\033[H", stdout);
}

VDev vdev_console_create(void) {
    VDev dev;
    memset(&dev, 0, sizeof(dev));
    dev.name = "CON:";
    dev.dev_class = VDCLASS_HID;
    dev.dev_version = "1.0";
    dev.dev_description = "Standard Console I/O Terminal";
    dev.ops.putc  = con_putc;
    dev.ops.puts  = con_puts;
    dev.ops.getc  = con_getc;
    dev.ops.gets  = con_gets;
    dev.ops.flush = con_flush;
    dev.ops.cls   = con_cls;
    dev.priv      = NULL;
    return dev;
}

#ifndef BPP_LITE_BUILD
extern void vdev_gfx_beep(VDevContext *ctx);
#endif

void vdev_play_beep(VDevContext *ctx) {
#ifdef BPP_LITE_BUILD
    vdev_putc(ctx, '\a');
#else
    vdev_gfx_beep(ctx);
#endif
}

#ifdef BPP_LITE_BUILD
void vdev_music_clear(void) {
    /* Stub for Lite builds */
}
#endif
