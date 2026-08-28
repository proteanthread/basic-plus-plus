// FILENAME: gfx_tui.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, hal.h, string.h)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (gfx_internal.h)
// NEEDS: libplatform (platform.h)
// Implements virtual device and graphics rendering logic for gfx_tui.
//
// ---- Includes ----

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device/gfx_internal.h"
#include "platform/platform.h"
#include "hal/hal.h"

char    g_screen_chars[MAX_GRID_ROWS][MAX_GRID_COLS];
uint8_t g_screen_attribs[MAX_GRID_ROWS][MAX_GRID_COLS];

static VDevOps original_con_ops;
static bool    con_ops_saved = false;

static int g_key_buffer[64];
static int g_key_head = 0;
static int g_key_tail = 0;

typedef struct {
    int x;
    int y;
    int left_button;
    int right_button;
    int middle_button;
    int wheel;
} MouseEventState;

#define MAX_MOUSE_QUEUE 64
static MouseEventState g_mouse_queue[MAX_MOUSE_QUEUE];
static int g_mouse_head = 0;
static int g_mouse_tail = 0;
static int g_mouse_size = 0;
static MouseEventState g_current_mouse_state = {0, 0, 0, 0, 0, 0};

static double g_last_tui_render_time = 0.0;
static bool   g_force_tui_render = false;

// 8x8 font grid representing ASCII characters 32 to 126
const uint8_t GW_FONT[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32: [space]
    {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00}, // 33: !
    {0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, // 34: "
    {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00}, // 35: #
    {0x08, 0x3E, 0x08, 0x3E, 0x08, 0x3E, 0x08, 0x00}, // 36: $
    {0x00, 0x62, 0x66, 0x0c, 0x18, 0x30, 0x46, 0x46}, // 37: %
    {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x7e, 0x00}, // 38: &
    {0x0c, 0x0c, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // 39: '
    {0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00}, // 40: (
    {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00}, // 41: )
    {0x00, 0x10, 0xd6, 0x38, 0xd6, 0x10, 0x00, 0x00}, // 42: *
    {0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00}, // 43: +
    {0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x04, 0x08}, // 44: ,
    {0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00}, // 45: -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // 46: .
    {0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}, // 47: /
    {0x3c, 0x66, 0x6e, 0x76, 0x66, 0x66, 0x3c, 0x00}, // 48: 0
    {0x18, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, // 49: 1
    {0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00}, // 50: 2
    {0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00}, // 51: 3
    {0x06, 0x0e, 0x1e, 0x66, 0x7e, 0x06, 0x06, 0x00}, // 52: 4
    {0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00}, // 53: 5
    {0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00}, // 54: 6
    {0x7e, 0x66, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00}, // 55: 7
    {0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00}, // 56: 8
    {0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00}, // 57: 9
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // 58: :
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10}, // 59: ;
    {0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00}, // 60: <
    {0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00}, // 61: =
    {0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00}, // 62: >
    {0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00}, // 63: ?
    {0x3c, 0x66, 0x6f, 0x7b, 0x73, 0x60, 0x3c, 0x00}, // 64: @
    {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00}, // 65: A
    {0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00}, // 66: B
    {0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00}, // 67: C
    {0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00}, // 68: D
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00}, // 69: E
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00}, // 70: F
    {0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3e, 0x00}, // 71: G
    {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00}, // 72: H
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, // 73: I
    {0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3c, 0x00}, // 74: J
    {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00}, // 75: L
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00}, // 76: M
    {0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00}, // 77: N
    {0x66, 0x76, 0x7e, 0x76, 0x6e, 0x66, 0x66, 0x00}, // 78: O
    {0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 79: P
    {0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00}, // 80: Q
    {0x3c, 0x66, 0x66, 0x66, 0x6a, 0x6c, 0x36, 0x00}, // 81: R
    {0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00}, // 82: S
    {0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00}, // 83: T
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 84: U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 85: V
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // 86: W
    {0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00}, // 87: X
    {0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00}, // 88: Y
    {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00}, // 89: Z
    {0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00}, // 90: [
    {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00}, // 91: backslash
    {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // 92: ]
    {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00}, // 93: ^
    {0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // 94: _
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00}, // 95: `
    {0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, // 96: a
    {0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00}, // 97: b
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00}, // 98: c
    {0x00, 0x00, 0x3c, 0x60, 0x60, 0x66, 0x3c, 0x00}, // 99: d
    {0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00}, // 100: e
    {0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00}, // 101: f
    {0x1c, 0x36, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00}, // 102: g
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // 103: h
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // 104: i
    {0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 105: j
    {0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x38}, // 106: k
    {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00}, // 107: l
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 108: m
    {0x00, 0x00, 0x6e, 0x7f, 0x6b, 0x63, 0x63, 0x00}, // 109: n
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // 110: o
    {0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00}, // 111: p
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60}, // 112: q
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06}, // 113: r
    {0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00}, // 114: s
    {0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x3c, 0x00}, // 115: t
    {0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00}, // 116: u
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00}, // 117: v
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // 118: w
    {0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00}, // 119: x
    {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00}, // 120: y
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c}, // 121: z
    {0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00}, // 122: {
    {0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00}, // 123: |
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // 124: }
    {0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00}, // 125: ~
    {0x76, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

//
// ---- Screen Buffer and Text Rendering ----

// scrolls the text buffer and pixel screen upward by one character row
void gfx_scroll_screen(void) {
    if (!g_pixels) return;
    int char_h = (graphics_mode == 13) ? 8 : 16;
    int pixel_rows_to_move = g_height - char_h;
    if (pixel_rows_to_move <= 0) return;

    memmove(g_pixels, g_pixels + char_h * g_width, pixel_rows_to_move * g_width * sizeof(uint32_t));

    uint32_t bg_color = (g_palette[g_bg_color_idx].b << 24) |
                        (g_palette[g_bg_color_idx].g << 16) |
                        (g_palette[g_bg_color_idx].r << 8) | 255;
    for (int i = pixel_rows_to_move * g_width; i < g_width * g_height; i++) {
        g_pixels[i] = bg_color;
    }

    for (int r = 0; r < MAX_GRID_ROWS - 1; ++r) {
        memcpy(g_screen_chars[r], g_screen_chars[r + 1], MAX_GRID_COLS);
        memcpy(g_screen_attribs[r], g_screen_attribs[r + 1], MAX_GRID_COLS);
    }
    memset(g_screen_chars[MAX_GRID_ROWS - 1], ' ', MAX_GRID_COLS);
    memset(g_screen_attribs[MAX_GRID_ROWS - 1], 7, MAX_GRID_COLS);
}

// draws a single character glyph at a specific grid position
void gfx_draw_char(char c, int col, int row) {
    if (row >= 0 && row < MAX_GRID_ROWS && col >= 0 && col < MAX_GRID_COLS) {
        g_screen_chars[row][col] = c;
        g_screen_attribs[row][col] = (uint8_t)(g_fg_color_idx | (g_bg_color_idx << 4));
    }

    if (!g_pixels) return;
    int char_w = 8;
    int char_h = (graphics_mode == 13) ? 8 : 16;

    int start_x = col * char_w;
    int start_y = row * char_h;

    const uint8_t *glyph = GW_FONT[0];
    if (c >= 32 && c <= 126) {
        glyph = GW_FONT[c - 32];
    }

    uint32_t fg = (g_palette[g_fg_color_idx].r << 24) |
                  (g_palette[g_fg_color_idx].g << 16) |
                  (g_palette[g_fg_color_idx].b << 8) | 255;
    uint32_t bg = (g_palette[g_bg_color_idx].r << 24) |
                  (g_palette[g_bg_color_idx].g << 16) |
                  (g_palette[g_bg_color_idx].b << 8) | 255;

    for (int r = 0; r < 8; r++) {
        uint8_t bits = glyph[r];
        int repeats = (char_h == 16) ? 2 : 1;
        for (int rep = 0; rep < repeats; rep++) {
            int py = start_y + r * repeats + rep;
            if (py < 0 || py >= g_height) continue;
            for (int col_idx = 0; col_idx < 8; col_idx++) {
                int px = start_x + col_idx;
                if (px < 0 || px >= g_width) continue;
                int bit = (bits >> (7 - col_idx)) & 1;
                g_pixels[py * g_width + px] = bit ? fg : bg;
            }
        }
    }
}

// outputs a character to the graphical console device
int gfx_con_putc(VDev *dev, int c) {
    (void)dev;
    if (c == '\n') {
        g_cursor_x = 0;
        g_cursor_y++;
        if (g_cursor_y >= g_grid_rows) {
            gfx_scroll_screen();
            g_cursor_y = g_grid_rows - 1;
        }
    } else if (c == '\r') {
        g_cursor_x = 0;
    } else if (c == '\t') {
        g_cursor_x = (g_cursor_x + 8) & ~7;
        if (g_cursor_x >= g_grid_cols) {
            g_cursor_x = 0;
            g_cursor_y++;
            if (g_cursor_y >= g_grid_rows) {
                gfx_scroll_screen();
                g_cursor_y = g_grid_rows - 1;
            }
        }
    } else if (c == '\b') {
        if (g_cursor_x > 0) {
            g_cursor_x--;
            gfx_draw_char(' ', g_cursor_x, g_cursor_y);
        }
    } else {
        gfx_draw_char((char)c, g_cursor_x, g_cursor_y);
        g_cursor_x++;
        if (g_cursor_x >= g_grid_cols) {
            g_cursor_x = 0;
            g_cursor_y++;
            if (g_cursor_y >= g_grid_rows) {
                gfx_scroll_screen();
                g_cursor_y = g_grid_rows - 1;
            }
        }
    }

    if (con_ops_saved && original_con_ops.putc) {
        original_con_ops.putc(dev, c);
    }
    return c;
}

// outputs a string to the graphical console device
int gfx_con_puts(VDev *dev, const char *s) {
    int count = 0;
    while (*s) {
        gfx_con_putc(dev, *s);
        s++;
        count++;
    }
    dev->ops.flush(dev);
    return count;
}

// presents the graphical framebuffer to BGI/HAL or falls back to TUI
int gfx_con_flush(VDev *dev) {
    (void)dev;
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_present(bgi);
        return 0;
    }
    if (g_pixels) {
        vdev_gfx_render_tui();
    }
    return 0;
}

// clears the graphical console screen and text matrix
int gfx_con_cls(VDev *dev) {
    (void)dev;
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_clearviewport(bgi);
        BGI_present(bgi);
    }
    memset(g_screen_chars, ' ', sizeof(g_screen_chars));
    memset(g_screen_attribs, 0, sizeof(g_screen_attribs));
    g_cursor_x = 0;
    g_cursor_y = 0;
    return 0;
}

// forces an immediate render pass to the terminal TUI
void vdev_gfx_force_flush(void) {
    g_force_tui_render = true;
    gfx_con_flush(NULL);
    g_force_tui_render = false;
}

// renders the current framebuffer using ANSI half-block characters
void vdev_gfx_render_tui(void) {
    if (!g_pixels || g_width <= 0 || g_height <= 0) return;

    double now = platform_get_timer();
    if (!g_force_tui_render && (now - g_last_tui_render_time < 0.066)) return;
    g_last_tui_render_time = now;

    int term_w = platform_console_width();
    int term_h = platform_console_height();
    if (term_w <= 0) term_w = 80;
    if (term_h <= 0) term_h = 25;

    int target_w = term_w;
    int target_h = (term_h - 2) * 2;
    if (target_h <= 0) target_h = 2;

    printf("\033[H");

    for (int ty = 0; ty < target_h; ty += 2) {
        for (int tx = 0; tx < target_w; ++tx) {
            int sx = tx * g_width / target_w;
            int sy1 = ty * g_height / target_h;
            int sy2 = (ty + 1) * g_height / target_h;

            if (sx >= g_width) sx = g_width - 1;
            if (sy1 >= g_height) sy1 = g_height - 1;
            if (sy2 >= g_height) sy2 = g_height - 1;

            uint32_t c1 = g_pixels[sy1 * g_width + sx];
            uint32_t c2 = g_pixels[sy2 * g_width + sx];

            uint8_t r1 = (c1 >> 24) & 0xFF;
            uint8_t g1 = (c1 >> 16) & 0xFF;
            uint8_t b1 = (c1 >> 8) & 0xFF;

            uint8_t r2 = (c2 >> 24) & 0xFF;
            uint8_t g2 = (c2 >> 16) & 0xFF;
            uint8_t b2 = (c2 >> 8) & 0xFF;

            printf("\033[48;2;%d;%d;%dm\033[38;2;%d;%d;%dm▄", r1, g1, b1, r2, g2, b2);
        }
        printf("\033[0m\n");
    }
    fflush(stdout);
}

// polls host window, keyboard, and mouse input events
void vdev_gfx_poll_events(void) {
    HalContext *hal = hal_get();
    if (hal && hal->video.poll_events) {
        hal->video.poll_events();
    }

    if (g_pixels) {
        vdev_gfx_render_tui();
    }

    while (platform_kbhit()) {
        int ch = platform_getch();
        if (ch > 0) {
            int next = (g_key_tail + 1) % 64;
            if (next != g_key_head) {
                g_key_buffer[g_key_tail] = ch;
                g_key_tail = next;
            }
        }
    }
}

// retrieves the next available keystroke from the ring buffer
int gfx_con_getc(VDev *dev) {
    (void)dev;
    while (true) {
        vdev_gfx_poll_events();
        if (g_key_head != g_key_tail) {
            int code = g_key_buffer[g_key_head];
            g_key_head = (g_key_head + 1) % 64;
            return code;
        }
        platform_sleep_ms(5);
    }
}

// reads a line of text input into the supplied buffer
char *gfx_con_gets(VDev *dev, char *buf, size_t size) {
    if (!buf || size == 0) return NULL;
    size_t count = 0;
    while (count + 1 < size) {
        int c = gfx_con_getc(dev);
        if (c == 13 || c == 10) {
            gfx_con_putc(dev, '\n');
            buf[count++] = '\n';
            break;
        } else if (c == 8) {
            if (count > 0) {
                count--;
                gfx_con_putc(dev, '\b');
            }
        } else if (c >= 32 && c <= 126) {
            gfx_con_putc(dev, c);
            buf[count++] = (char)c;
        }
    }
    buf[count] = '\0';
    return buf;
}
