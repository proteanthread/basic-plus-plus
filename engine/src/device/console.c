// FILENAME: console.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (hal.h, logger.h, logger.c, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libengine (bios.h, bios.c)
// NEEDS: libkernel (vcon.h, vcon.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Implements virtual device and graphics rendering logic for console.
//
// ---- Includes ----

#include "device/vdev.h"
#include "device/vcon.h"
#include "bios/bios.h"
#include "debug/logger.h"
#include "platform/platform.h"
#include <stdint.h>
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

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

static void raw_console_write(const char *s, size_t len) {
    if (!s || len == 0) return;
    HalContext *hal = hal_get();
    if (hal) {
        if (hal->io.file_write) {
            hal->io.file_write(IO_STDOUT_HANDLE, s, 1, len);
        } else if (hal->io.console_putchar) {
            for (size_t i = 0; i < len; i++) {
                hal->io.console_putchar((unsigned char)s[i]);
            }
        }
    }
}

static void raw_console_flush(void) {
    HalContext *hal = hal_get();
    if (hal && hal->io.console_flush) {
        hal->io.console_flush();
    }
}


void console_hide_mouse_cursor(void) {
    if (!g_cursor_drawn) return;
    if (g_saved_mouse_row > 0 && g_saved_mouse_col > 0) {
        int fg_ansi = 37, bg_ansi = 40;
        get_ansi_colors_from_attrib(g_saved_attr, &fg_ansi, &bg_ansi);
        char buf[128];
        int n = runtime_snprintf(buf, sizeof(buf), "\033[s\033[%d;%dH\033[%d;%dm%c\033[0m\033[u", 
                                 g_saved_mouse_row, g_saved_mouse_col, fg_ansi, bg_ansi, g_saved_char);
        if (n > 0) raw_console_write(buf, (size_t)n);
        raw_console_flush();
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
    
    char buf[128];
    int n = runtime_snprintf(buf, sizeof(buf), "\033[s\033[%d;%dH\033[%d;%dm%c\033[0m\033[u", 
                             row, col, fg_ansi, bg_ansi, final_char);
    if (n > 0) raw_console_write(buf, (size_t)n);
    raw_console_flush();
    
    g_cursor_drawn = true;
}

static int con_putc(VDev *dev, int c) {
    (void)dev;
    console_hide_mouse_cursor();
    extern VConContext *g_vcon_context;
    extern BiosContext *g_bios_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        // Capture cursor position BEFORE vcon_write_char advances it
        int cur_row = 0, cur_col = 0;
        vcon_get_cursor(g_vcon_context, active_idx, &cur_row, &cur_col);
        vcon_write_char(g_vcon_context, active_idx, c);
        // Sync to BIOS VRAM backing store (raw, no observer trigger)
        if (g_bios_context && c >= 0x20 && c <= 0x7E) {
            uint8_t attr = (uint8_t)vcon_get_attr_at(g_vcon_context, active_idx, cur_row, cur_col);
            uint32_t vram_addr = BIOS_VRAM_CGA_START + (uint32_t)(cur_row * 160 + cur_col * 2);
            bios_poke_raw(g_bios_context, vram_addr, (uint8_t)c);
            bios_poke_raw(g_bios_context, vram_addr + 1, attr);
        }
    }
    char c_str[2] = {(char)c, '\0'};
    log_write_out(c_str, 1);
    raw_console_write(c_str, 1);
    console_draw_mouse_cursor();
    return c;
}

static int con_puts(VDev *dev, const char *s) {
    (void)dev;
    console_hide_mouse_cursor();
    extern VConContext *g_vcon_context;
    extern BiosContext *g_bios_context;
    if (g_vcon_context && s) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        const char *p = s;
        while (*p) {
            unsigned char uc = (unsigned char)*p++;
            // Capture cursor position BEFORE vcon_write_char advances it
            int cur_row = 0, cur_col = 0;
            vcon_get_cursor(g_vcon_context, active_idx, &cur_row, &cur_col);
            vcon_write_char(g_vcon_context, active_idx, uc);
            // Sync to BIOS VRAM backing store (raw, no observer trigger)
            if (g_bios_context && uc >= 0x20 && uc <= 0x7E) {
                uint8_t attr = (uint8_t)vcon_get_attr_at(g_vcon_context, active_idx, cur_row, cur_col);
                uint32_t vram_addr = BIOS_VRAM_CGA_START + (uint32_t)(cur_row * 160 + cur_col * 2);
                bios_poke_raw(g_bios_context, vram_addr, uc);
                bios_poke_raw(g_bios_context, vram_addr + 1, attr);
            }
        }
    }
    if (s) {
        size_t len = runtime_strlen(s);
        log_write_out(s, len);
        raw_console_write(s, len);
    }
    console_draw_mouse_cursor();
    return 0;
}

static int con_getc(VDev *dev) {
    (void)dev;
    HalContext *hal = hal_get();
    if (hal && hal->io.console_getchar) {
        return hal->io.console_getchar();
    }
    return -1;
}

static char *con_gets(VDev *dev, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;

    if (!platform_stdin_is_console()) {
        HalContext *hal = hal_get();
        if (hal && hal->io.file_read) {
            size_t idx = 0;
            while (idx < size - 1) {
                char c = 0;
                size_t read_bytes = hal->io.file_read(IO_STDIN_HANDLE, &c, 1, 1);
                if (read_bytes == 0) break;
                buf[idx++] = c;
                if (c == '\n') break;
            }
            if (idx == 0) return NULL;
            buf[idx] = '\0';
            return buf;
        }
        return NULL;
    }

    HalContext *hal = hal_get();
    if (!hal || !hal->io.console_getchar) return NULL;

    size_t idx = 0;
    while (idx < size - 1) {
        int ch = hal->io.console_getchar();
        if (ch == -1 || ch == 3 || ch == 4 || ch == 26) {
            if (idx == 0) return NULL;
            break;
        }
        if (ch == '\r' || ch == '\n') {
            buf[idx++] = '\n';
            con_putc(dev, '\n');
            break;
        }
        if (ch == '\b' || ch == 127) {
            if (idx > 0) {
                idx--;
                raw_console_write("\b \b", 3);
                raw_console_flush();
            }
            continue;
        }
        if (ch == 224 || ch == 0) {
            hal->io.console_getchar();
            continue;
        }
        if (ch >= 32 && ch < 127) {
            buf[idx++] = (char)ch;
            con_putc(dev, ch);
            raw_console_flush();
        }
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}


static int con_flush(VDev *dev) {
    (void)dev;
    console_draw_mouse_cursor();
    raw_console_flush();
    return 0;
}

static int con_cls(VDev *dev) {
    (void)dev;
    console_hide_mouse_cursor();
    extern VConContext *g_vcon_context;
    extern BiosContext *g_bios_context;
    if (g_vcon_context) {
        int active_idx = vcon_get_active_index(g_vcon_context);
        vcon_clear(g_vcon_context, active_idx);
    }
    // Clear BIOS CGA text VRAM backing store: fill with space (0x20) + default attr (0x07)
    if (g_bios_context) {
        for (uint32_t i = 0; i < 4000; i += 2) {
            bios_poke_raw(g_bios_context, BIOS_VRAM_CGA_START + i, 0x20);
            bios_poke_raw(g_bios_context, BIOS_VRAM_CGA_START + i + 1, 0x07);
        }
    }
    // ANSI escape to clear screen and reset cursor
    raw_console_write("\033[2J\033[H", 7);
    console_draw_mouse_cursor();
    return 0;
}

VDev vdev_console_create(void) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));
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

#ifndef BASIC_LITE_BUILD
extern void vdev_gfx_beep(VDevContext *ctx);
#endif

void vdev_play_beep(VDevContext *ctx) {
#ifdef BASIC_LITE_BUILD
    vdev_putc(ctx, '\a');
#else
    vdev_gfx_beep(ctx);
#endif
}

