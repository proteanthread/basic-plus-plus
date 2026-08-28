// FILENAME: gfx_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (gfx.c, gfx_audio.c, gfx_palette.c)
// NEEDED BY: libkernel (gfx_primitives_internal.h, gfx_tui.c)
// NEEDS: libengine (bgi.h, bgi.c, eval.h, eval.c, lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Implements virtual device and graphics rendering logic for gfx_internal.
//
// ---- Includes ----

#ifndef DEVICE_GFX_INTERNAL_H
#define DEVICE_GFX_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "device/bgi.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

// Freestanding 32-bit RGBA color representation
typedef struct BppColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} BppColor;

//
// ---- Shared Grid and Dimensions ----

#define MAX_GRID_ROWS 40
#define MAX_GRID_COLS 100

extern uint32_t   *g_pixels;
extern int         g_width;
extern int         g_height;
extern int         graphics_mode;
extern int         g_fg_color_idx;
extern int         g_bg_color_idx;
extern int         g_cursor_x;
extern int         g_cursor_y;
extern int         g_grid_cols;
extern int         g_grid_rows;
extern char        g_screen_chars[MAX_GRID_ROWS][MAX_GRID_COLS];
extern uint8_t     g_screen_attribs[MAX_GRID_ROWS][MAX_GRID_COLS];
extern BppColor    g_palette[256];
extern bool        g_graphics_allowed;
extern bool        g_sdl_gui_boot;
extern bool        g_gfx_quit_requested;

extern int         g_mouse_x;
extern int         g_mouse_y;
extern int         g_mouse_btn;

extern const uint8_t GW_FONT[96][8];

//
// ---- Internal Function Prototypes ----

void init_palette(void);
void bgi_sync_screen_mode(int screen_mode);
void bgi_shutdown_mode(void);
uint32_t screen_mode_to_bgi_id(int screen_mode);

void gfx_pset(int x, int y, uint32_t col);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void draw_circle(int cx, int cy, int r, uint32_t color);
BppError init_graphics_mode_dims(VMContext *vm, int width, int height);

int gfx_con_putc(VDev *dev, int c);
int gfx_con_puts(VDev *dev, const char *s);
int gfx_con_flush(VDev *dev);
int gfx_con_cls(VDev *dev);
int gfx_con_getc(VDev *dev);
char *gfx_con_gets(VDev *dev, char *buf, size_t size);
void gfx_scroll_screen(void);
void gfx_draw_char(char c, int col, int row);
void vdev_gfx_render_tui(void);
void vdev_gfx_poll_events(void);

bool init_audio(void);
void push_note(double frequency, double duration_seconds, bool background);
void play_mml(VMContext *vm, const char *mml);

BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_screen_mode_handler(VMContext *vm, int mode);

#endif // DEVICE_GFX_INTERNAL_H
