/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

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

#include "device/vdev.h"
#include "device/vcon.h"
#include "debug/logger.h"
#include <stdio.h>
#include <string.h>

extern bool platform_mouse_is_visible(void);
extern void platform_mouse_get_position(int *col, int *row);
extern int platform_console_width(void);
extern int platform_console_height(void);
extern void platform_mouse_get_cursor(int *char_code, int *attrib);

static int g_saved_mouse_col = -1;
static int g_saved_mouse_row = -1;
static int g_saved_char = 32;
static int g_saved_attr = 7;
static bool g_cursor_drawn = false;

static void get_ansi_colors_from_attrib(int attr, int *fg_ansi, int *bg_ansi) {
    int fg = attr & 0x0F;
    int bg = (attr >> 4) & 0x0F;
    static const int fg_map[] = { 30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97 };
    static const int bg_map[] = { 40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107 };
    if (fg_ansi) *fg_ansi = fg_map[fg];
    if (bg_ansi) *bg_ansi = bg_map[bg];
}

void console_hide_mouse_cursor(void) {
    if (!g_cursor_drawn) return;
    if (g_saved_mouse_row > 0 && g_saved_mouse_col > 0) {
        int fg_ansi = 37, bg_ansi = 40;
        get_ansi_colors_from_attrib(g_saved_attr, &fg_ansi, &bg_ansi);
        printf("\033[s\033[%d;%dH\033[%d;%dm%c\033[0m\033[u", 
               g_saved_mouse_row, g_saved_mouse_col, fg_ansi, bg_ansi, g_saved_char);
        fflush(stdout);
    }
    g_cursor_drawn = false;
}

void console_draw_mouse_cursor(void) {
    if (!platform_mouse_is_visible()) return;
    if (g_cursor_drawn) return;
    
    int col = 1, row = 1;
    platform_mouse_get_position(&col, &row);
    
    if (col < 1 || col > platform_console_width() || row < 1 || row > platform_console_height()) {
        return;
    }
    
    int hover_char = 32;
    int hover_attr = 7;
    extern VConContext *g_vcon_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        hover_char = vcon_get_char_at(g_vcon_context, active_idx, row - 1, col - 1);
        hover_attr = vcon_get_attr_at(g_vcon_context, active_idx, row - 1, col - 1);
    }
    
    g_saved_mouse_col = col;
    g_saved_mouse_row = row;
    g_saved_char = hover_char;
    g_saved_attr = hover_attr;
    
    int cursor_char = 0;
    int cursor_attr = 0;
    platform_mouse_get_cursor(&cursor_char, &cursor_attr);
    
    int final_char = (cursor_char > 0) ? cursor_char : hover_char;
    int final_attr = hover_attr;
    if (cursor_attr > 0) {
        final_attr = cursor_attr;
    } else {
        final_attr = ((hover_attr & 0x0F) << 4) | ((hover_attr >> 4) & 0x0F);
    }
    
    int fg_ansi = 37, bg_ansi = 40;
    get_ansi_colors_from_attrib(final_attr, &fg_ansi, &bg_ansi);
    
    printf("\033[s\033[%d;%dH\033[%d;%dm%c\033[0m\033[u", 
           row, col, fg_ansi, bg_ansi, final_char);
    fflush(stdout);
    
    g_cursor_drawn = true;
}

static int con_putc(VDev *dev, int c) {
    (void)dev;
    console_hide_mouse_cursor();
    extern VConContext *g_vcon_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        vcon_write_char(g_vcon_context, active_idx, c);
    }
    char c_str[2] = {(char)c, '\0'};
    bpp_log_write_out(c_str, 1);
    int res = putchar(c);
    console_draw_mouse_cursor();
    return res;
}

static int con_puts(VDev *dev, const char *s) {
    (void)dev;
    console_hide_mouse_cursor();
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
    console_draw_mouse_cursor();
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
    console_draw_mouse_cursor();
    return fflush(stdout);
}

static int con_cls(VDev *dev) {
    (void)dev;
    console_hide_mouse_cursor();
    extern VConContext *g_vcon_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        vcon_clear(g_vcon_context, active_idx);
    }
    /* ANSI escape to clear screen and reset cursor */
    int res = fputs("\033[2J\033[H", stdout);
    console_draw_mouse_cursor();
    return res;
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
