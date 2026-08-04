/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file vdev_gfx.c
 * @brief SDL2 Graphics and Audio virtual device and statement handlers.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements SCREEN, COLOR, LINE, CIRCLE, PSET, PRESET, CLS, PAINT,
 *   SOUND, and PLAY statements. It supports both pure console (NO_SDL2) and SDL2 builds.
 * - Why it exists: Provides classic graphics modes (e.g. SCREEN 12, SCREEN 13) and
 *   graphical drawing primitives, decoupling them from direct system graphics drivers.
 * - Why it works this way: It uses a software pixel framebuffer (g_pixels) that updates
 *   a streaming texture on rendering present calls. It dynamically overrides the "CON:"
 *   virtual device callbacks to render text output onto the window canvas.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Color palettes, window resolutions, text grid dimensions.
 * - What cannot be changed: Callback signatures for VDevOps and statement handler signatures.
 * - What to expect: Entering a graphics mode overrides screen puts/putc console streams.
 * - What to do if something breaks: Verify SDL event polling execution frequency and memory allocations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard character sets are ASCII. Font is 8x8 pixels per glyph.
 * - Portability concerns: SDL2 includes differ between Windows (standard headers) and Linux (pkg-config).
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add mouse/keyboard events notifications to the VM.
 * - How to write external extensions: Custom screen driver extensions hook into SCREEN/CLS calls.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "device/bgi.h"
#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex);

#ifndef NO_SDL2
#include <SDL.h>

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
#endif

extern bool platform_kbhit(void);
extern int  platform_getch(void);

static bool g_graphics_allowed = false;
static bool g_sdl_gui_boot = false;
bool g_gfx_quit_requested = false;

void vdev_gfx_enable(bool allowed, bool gui_boot) {
    g_graphics_allowed = allowed;
    g_sdl_gui_boot = gui_boot;
}

void vdev_gfx_boot_check(VMContext *vm) {
#ifndef NO_SDL2
    if (g_graphics_allowed && g_sdl_gui_boot) {
        BppError err = vm_execute_line(vm, "SCREEN 12");
        (void)err;
    }
#else
    (void)vm;
#endif
}

#ifdef NO_SDL2
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} SDL_Color;
#else
#include <SDL.h>
#endif

#ifndef NO_SDL2
static SDL_Window   *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture  *sdl_texture = NULL;
#endif

static uint32_t     *g_pixels = NULL;
static int           g_width = 640;
static int           g_height = 480;
static int           graphics_mode = 0;
extern int           g_fg_color_idx;
extern int           g_bg_color_idx;
#define fg_color_idx g_fg_color_idx
#define bg_color_idx g_bg_color_idx

extern int           g_cursor_x;
extern int           g_cursor_y;
static int           g_grid_cols = 80;
static int           g_grid_rows = 30;

#define MAX_GRID_ROWS 40
#define MAX_GRID_COLS 100
static char g_screen_chars[MAX_GRID_ROWS][MAX_GRID_COLS];
static uint8_t g_screen_attribs[MAX_GRID_ROWS][MAX_GRID_COLS];

extern int           g_mouse_x;
extern int           g_mouse_y;
extern int           g_mouse_btn;

static SDL_Color     palette[256];

static VDevOps       original_con_ops;
static bool          con_ops_saved = false;

/* Keyboard Ring Buffer */
static int           g_key_buffer[64];
static int           g_key_head = 0;
static int           g_key_tail = 0;

/* Classic 8x8 font grid representing ASCII 32 to 126 */
static const uint8_t GW_FONT[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 32: [space] */
    {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00}, /* 33: ! */
    {0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 34: " */
    {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00}, /* 35: # */
    {0x08, 0x3E, 0x08, 0x3E, 0x08, 0x3E, 0x08, 0x00}, /* 36: $ */
    {0x00, 0x62, 0x66, 0x0c, 0x18, 0x30, 0x46, 0x46}, /* 37: % */
    {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x7e, 0x00}, /* 38: & */
    {0x0c, 0x0c, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 39: ' */
    {0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00}, /* 40: ( */
    {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00}, /* 41: ) */
    {0x00, 0x10, 0xd6, 0x38, 0xd6, 0x10, 0x00, 0x00}, /* 42: * */
    {0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00}, /* 43: + */
    {0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x04, 0x08}, /* 44: , */
    {0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00}, /* 45: - */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, /* 46: . */
    {0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}, /* 47: / */
    {0x3c, 0x66, 0x6e, 0x76, 0x66, 0x66, 0x3c, 0x00}, /* 48: 0 */
    {0x18, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, /* 49: 1 */
    {0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00}, /* 50: 2 */
    {0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00}, /* 51: 3 */
    {0x06, 0x0e, 0x1e, 0x66, 0x7e, 0x06, 0x06, 0x00}, /* 52: 4 */
    {0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00}, /* 53: 5 */
    {0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00}, /* 54: 6 */
    {0x7e, 0x66, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00}, /* 55: 7 */
    {0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00}, /* 56: 8 */
    {0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00}, /* 57: 9 */
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, /* 58: : */
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10}, /* 59: ; */
    {0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00}, /* 60: < */
    {0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00}, /* 61: = */
    {0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00}, /* 62: > */
    {0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00}, /* 63: ? */
    {0x3c, 0x66, 0x6f, 0x7b, 0x73, 0x60, 0x3c, 0x00}, /* 64: @ */
    {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00}, /* 65: A */
    {0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00}, /* 66: B */
    {0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00}, /* 67: C */
    {0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00}, /* 68: D */
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7e, 0x00}, /* 69: E */
    {0x7e, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00}, /* 70: F */
    {0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3e, 0x00}, /* 71: G */
    {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00}, /* 72: H */
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00}, /* 73: I */
    {0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3c, 0x00}, /* 74: J */
    {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00}, /* 75: L */
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00}, /* 76: M */
    {0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00}, /* 77: N */
    {0x66, 0x76, 0x7e, 0x76, 0x6e, 0x66, 0x66, 0x00}, /* 78: O */
    {0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, /* 79: P */
    {0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00}, /* 80: Q */
    {0x3c, 0x66, 0x66, 0x66, 0x6a, 0x6c, 0x36, 0x00}, /* 81: R */
    {0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00}, /* 82: S */
    {0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00}, /* 83: T */
    {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, /* 84: U */
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00}, /* 85: V */
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, /* 86: W */
    {0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00}, /* 87: X */
    {0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00}, /* 88: Y */
    {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00}, /* 89: Z */
    {0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00}, /* 90: [ */
    {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00}, /* 91: backslash */
    {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, /* 92: ] */
    {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00}, /* 93: ^ */
    {0x08, 0x1c, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, /* 94: _ */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00}, /* 95: ` */
    {0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, /* 96: a */
    {0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00}, /* 97: b */
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00}, /* 98: c */
    {0x00, 0x00, 0x3c, 0x60, 0x60, 0x66, 0x3c, 0x00}, /* 99: d */
    {0x06, 0x06, 0x3e, 0x66, 0x66, 0x66, 0x3e, 0x00}, /* 100: e */
    {0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00}, /* 101: f */
    {0x1c, 0x36, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00}, /* 102: g */
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c}, /* 103: h */
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, /* 104: i */
    {0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, /* 105: j */
    {0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x38}, /* 106: k */
    {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00}, /* 107: l */
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, /* 108: m */
    {0x00, 0x00, 0x6e, 0x7f, 0x6b, 0x63, 0x63, 0x00}, /* 109: n */
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, /* 110: o */
    {0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00}, /* 111: p */
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60}, /* 112: q */
    {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06}, /* 113: r */
    {0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00}, /* 114: s */
    {0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x3c, 0x00}, /* 115: t */
    {0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00}, /* 116: u */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00}, /* 117: v */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, /* 118: w */
    {0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00}, /* 119: x */
    {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00}, /* 120: y */
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c}, /* 121: z */
    {0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00}, /* 122: { */
    {0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00}, /* 123: | */
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, /* 124: } */
    {0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00}, /* 125: ~ */
    {0x76, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static void init_palette(void) {
    SDL_Color std_colors[16] = {
        {0, 0, 0, 255},       /* 0: Black */
        {0, 0, 170, 255},     /* 1: Blue */
        {0, 170, 0, 255},     /* 2: Green */
        {0, 170, 170, 255},   /* 3: Cyan */
        {170, 0, 0, 255},     /* 4: Red */
        {170, 0, 170, 255},   /* 5: Magenta */
        {170, 85, 0, 255},    /* 6: Brown */
        {170, 170, 170, 255}, /* 7: Light Gray */
        {85, 85, 85, 255},    /* 8: Dark Gray */
        {85, 85, 255, 255},   /* 9: Light Blue */
        {85, 255, 85, 255},   /* 10: Light Green */
        {85, 255, 255, 255},  /* 11: Light Cyan */
        {255, 85, 85, 255},   /* 12: Light Red */
        {255, 85, 255, 255},  /* 13: Light Magenta */
        {255, 255, 85, 255},  /* 14: Yellow */
        {255, 255, 255, 255}  /* 15: Bright White */
    };
    memcpy(palette, std_colors, sizeof(std_colors));
}

/* ======================================================================
 * BGI Integration Layer — Maps SCREEN modes to BGI mode IDs.
 * BGI is the primary renderer; SDL2 provides the display window.
 * ====================================================================== */

/** Map a BASIC SCREEN number to a BGI heritage mode ID. Returns 0 if no mapping. */
static uint32_t screen_mode_to_bgi_id(int screen_mode) {
    switch (screen_mode) {
        case 1:   return BGI_MODE_CGA_1;
        case 2:   return BGI_MODE_CGA_2;
        case 3:   return BGI_MODE_TANDY_3;
        case 9:   return BGI_MODE_EGA_9;
        case 10:  return BGI_MODE_EGA_10;
        case 12:  return BGI_MODE_VGA_12;
        case 13:  return BGI_MODE_VGA_13;
        default:  return 0; /* No BGI mapping */
    }
}

/**
 * @brief Initialize BGI for the given SCREEN mode.
 * BGI is the primary renderer. If BGI fails, SDL2 g_pixels is the fallback.
 */
static void bgi_sync_screen_mode(int screen_mode) {
    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized) {
        BGI_init(ctx);
    }
    uint32_t mode_id = screen_mode_to_bgi_id(screen_mode);
    if (mode_id != 0) {
        BGI_set_mode_by_id(ctx, mode_id);
    }
}

/**
 * @brief Sync the BGI framebuffer to the SDL2 g_pixels buffer.
 * Called after BGI drawing operations so SDL2 can display the result.
 */
static void bgi_sync_to_sdl(void) {
    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer || !g_pixels) return;
    if (ctx->fb_width != g_width || ctx->fb_height != g_height) return;

    /* Synthesize indexed VRAM to ARGB if needed */
    BGI_synthesize(ctx);

    /* Convert from BGI ARGB8888 (0xAARRGGBB) to SDL RGBA8888 (RRGGBBAA) */
    size_t total = (size_t)g_width * (size_t)g_height;
    for (size_t i = 0; i < total; ++i) {
        uint32_t argb = ctx->framebuffer[i];
        uint8_t r = (uint8_t)((argb >> 16) & 0xFF);
        uint8_t g = (uint8_t)((argb >>  8) & 0xFF);
        uint8_t b = (uint8_t)( argb        & 0xFF);
        g_pixels[i] = ((uint32_t)r << 24) | ((uint32_t)g << 16) |
                      ((uint32_t)b << 8) | 0xFFu;
    }
}

/**
 * @brief Shutdown BGI when returning to text mode (SCREEN 0).
 */
static void bgi_shutdown_mode(void) {
    BGI_Context *ctx = BGI_get_global_context();
    if (ctx->initialized) {
        BGI_shutdown(ctx);
    }
}

static void gfx_scroll_screen(void) {
    if (!g_pixels) return;
    int char_h = (graphics_mode == 13) ? 8 : 16;
    int pixel_rows_to_move = g_height - char_h;
    if (pixel_rows_to_move <= 0) return;
    
    memmove(g_pixels, g_pixels + char_h * g_width, pixel_rows_to_move * g_width * sizeof(uint32_t));
    
    uint32_t bg_color = (palette[bg_color_idx].b << 24) | 
                        (palette[bg_color_idx].g << 16) | 
                        (palette[bg_color_idx].r << 8) | 255;
    for (int i = pixel_rows_to_move * g_width; i < g_width * g_height; i++) {
        g_pixels[i] = bg_color;
    }

    /* Scroll character matrix */
    for (int r = 0; r < MAX_GRID_ROWS - 1; ++r) {
        memcpy(g_screen_chars[r], g_screen_chars[r + 1], MAX_GRID_COLS);
        memcpy(g_screen_attribs[r], g_screen_attribs[r + 1], MAX_GRID_COLS);
    }
    memset(g_screen_chars[MAX_GRID_ROWS - 1], ' ', MAX_GRID_COLS);
    memset(g_screen_attribs[MAX_GRID_ROWS - 1], 7, MAX_GRID_COLS);
}

static void gfx_draw_char(char c, int col, int row) {
    if (row >= 0 && row < MAX_GRID_ROWS && col >= 0 && col < MAX_GRID_COLS) {
        g_screen_chars[row][col] = c;
        g_screen_attribs[row][col] = (uint8_t)(fg_color_idx | (bg_color_idx << 4));
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
    
    /* RGBA format mapping depending on endianness. We use standard 0xRRGGBBAA or reverse. */
    uint32_t fg = (palette[fg_color_idx].r << 24) | 
                  (palette[fg_color_idx].g << 16) | 
                  (palette[fg_color_idx].b << 8) | 255;
    uint32_t bg = (palette[bg_color_idx].r << 24) | 
                  (palette[bg_color_idx].g << 16) | 
                  (palette[bg_color_idx].b << 8) | 255;
    
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

static int gfx_con_putc(VDev *dev, int c) {
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
    return c;
}

static int gfx_con_puts(VDev *dev, const char *s) {
    int count = 0;
    while (*s) {
        gfx_con_putc(dev, *s);
        s++;
        count++;
    }
    dev->ops.flush(dev);
    return count;
}

/* Forward declaration for TUI fallback used in gfx_con_flush */
void vdev_gfx_render_tui(void);

static int gfx_con_flush(VDev *dev) {
    (void)dev;
#ifndef NO_SDL2
    if (sdl_renderer && sdl_texture && g_pixels) {
        SDL_UpdateTexture(sdl_texture, NULL, g_pixels, g_width * sizeof(uint32_t));
        SDL_RenderClear(sdl_renderer);
        SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
        SDL_RenderPresent(sdl_renderer);
        return 0;
    }
#endif
    /* TUI half-block fallback when SDL is not active */
    if (g_pixels) {
        vdev_gfx_render_tui();
    }
    return 0;
}

static int gfx_con_cls(VDev *dev) {
    (void)dev;
    if (g_pixels) {
        uint32_t bg = (palette[bg_color_idx].r << 24) | 
                      (palette[bg_color_idx].g << 16) | 
                      (palette[bg_color_idx].b << 8) | 255;
        for (int i = 0; i < g_width * g_height; i++) {
            g_pixels[i] = bg;
        }
        g_cursor_x = 0;
        g_cursor_y = 0;

        for (int r = 0; r < MAX_GRID_ROWS; ++r) {
            memset(g_screen_chars[r], ' ', MAX_GRID_COLS);
            memset(g_screen_attribs[r], 7, MAX_GRID_COLS);
        }

        gfx_con_flush(dev);
    }
    return 0;
}

static double g_last_tui_render_time = 0.0;
static bool   g_force_tui_render = false;
double platform_get_timer(void);

void vdev_gfx_force_flush(void) {
    g_force_tui_render = true;
    gfx_con_flush(NULL);
    g_force_tui_render = false;
}

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

void vdev_gfx_poll_events(void) {
#ifndef NO_SDL2
    if (sdl_window) {
        int mx = 0, my = 0;
        uint32_t buttons = SDL_GetMouseState(&mx, &my);
        g_mouse_x = mx;
        g_mouse_y = my;
        g_mouse_btn = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                if (sdl_renderer) { SDL_DestroyRenderer(sdl_renderer); sdl_renderer = NULL; }
                if (sdl_texture) { SDL_DestroyTexture(sdl_texture); sdl_texture = NULL; }
                if (sdl_window) { SDL_DestroyWindow(sdl_window); sdl_window = NULL; }
                if (g_pixels) { free(g_pixels); g_pixels = NULL; }
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
                g_gfx_quit_requested = true;
                return;
            } else if (event.type == SDL_TEXTINPUT) {
                for (int i = 0; event.text.text[i] != '\0'; i++) {
                    char c = event.text.text[i];
                    int next = (g_key_tail + 1) % 64;
                    if (next != g_key_head) {
                        g_key_buffer[g_key_tail] = c;
                        g_key_tail = next;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode sym = event.key.keysym.sym;
                int code = 0;
                if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                    code = 13;
                } else if (sym == SDLK_BACKSPACE) {
                    code = 8;
                } else if (sym == SDLK_ESCAPE) {
                    code = 27;
                } else if (sym == SDLK_TAB) {
                    code = 9;
                }
                
                if (code > 0) {
                    int next = (g_key_tail + 1) % 64;
                    if (next != g_key_head) {
                        g_key_buffer[g_key_tail] = code;
                        g_key_tail = next;
                    }
                }
            } else if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEWHEEL) {
                int ev_mx = 0, ev_my = 0;
                uint32_t ev_buttons = SDL_GetMouseState(&ev_mx, &ev_my);
                MouseEventState me;
                me.x = ev_mx;
                me.y = ev_my;
                me.left_button = (ev_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
                me.right_button = (ev_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0;
                me.middle_button = (ev_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0;
                me.wheel = (event.type == SDL_MOUSEWHEEL) ? event.wheel.y : 0;

                if (g_mouse_size < MAX_MOUSE_QUEUE) {
                    g_mouse_queue[g_mouse_tail] = me;
                    g_mouse_tail = (g_mouse_tail + 1) % MAX_MOUSE_QUEUE;
                    g_mouse_size++;
                }
            }
        }
        return;
    }
#endif

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

static int gfx_con_getc(VDev *dev) {
    (void)dev;
    while (true) {
        vdev_gfx_poll_events();
        if (g_key_head != g_key_tail) {
            int code = g_key_buffer[g_key_head];
            g_key_head = (g_key_head + 1) % 64;
            return code;
        }
        SDL_Delay(5);
    }
}

static char *gfx_con_gets(VDev *dev, char *buf, size_t size) {
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

static void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        if (x1 >= 0 && x1 < g_width && y1 >= 0 && y1 < g_height) {
            g_pixels[y1 * g_width + x1] = color;
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

static void draw_circle(int cx, int cy, int r, uint32_t color) {
    int x = r;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        #define SET_PIX(px, py) do { \
            if ((px) >= 0 && (px) < g_width && (py) >= 0 && (py) < g_height) { \
                g_pixels[(py) * g_width + (px)] = color; \
            } \
        } while(0)
        
        SET_PIX(cx + x, cy + y);
        SET_PIX(cx + y, cy + x);
        SET_PIX(cx - y, cy + x);
        SET_PIX(cx - x, cy + y);
        SET_PIX(cx - x, cy - y);
        SET_PIX(cx - y, cy - x);
        SET_PIX(cx + y, cy - x);
        SET_PIX(cx + x, cy - y);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}


static BppError init_graphics_mode_dims(VMContext *vm, int width, int height) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (g_pixels) { free(g_pixels); g_pixels = NULL; }
    g_pixels = (uint32_t *)calloc(width * height, sizeof(uint32_t));
    if (!g_pixels) {
        err.code = 14; err.message = "Out of memory allocating software framebuffer";
        return err;
    }

    g_width = width;
    g_height = height;
    g_grid_cols = width / 8;
    g_grid_rows = height / 8;
    init_palette();

#ifdef NO_SDL2
    (void)vm;
    return err;
#else
    if (!g_graphics_allowed) {
        return err;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        err.code = 5; err.message = "Failed to initialize SDL Video subsystem";
        return err;
    }

    if (sdl_texture) { SDL_DestroyTexture(sdl_texture); sdl_texture = NULL; }
    if (sdl_renderer) { SDL_DestroyRenderer(sdl_renderer); sdl_renderer = NULL; }
    if (sdl_window) { SDL_DestroyWindow(sdl_window); sdl_window = NULL; }

    sdl_window = SDL_CreateWindow("BASIC++ Graphics Screen",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  g_width, g_height, SDL_WINDOW_SHOWN);
    if (!sdl_window) {
        err.code = 5; err.message = "Failed to create SDL window";
        return err;
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdl_renderer) {
        sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!sdl_renderer) {
        err.code = 5; err.message = "Failed to create SDL renderer";
        return err;
    }

    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                    SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    if (!sdl_texture) {
        err.code = 5; err.message = "Failed to create SDL texture";
        return err;
    }
#endif

    g_cursor_x = 0;
    g_cursor_y = 0;

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) {
        if (!con_ops_saved) {
            original_con_ops = con->ops;
            con_ops_saved = true;
        }
        con->ops.putc = gfx_con_putc;
        con->ops.puts = gfx_con_puts;
        con->ops.cls  = gfx_con_cls;
        con->ops.flush = gfx_con_flush;
        con->ops.getc = gfx_con_getc;
    }

    return err;
}

/**
 * @brief SCREEN statement handler.
 * Syntax: SCREEN mode
 */
BppError stmt_screen_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int mode = (int)mode_val.as.number;

#ifdef NO_SDL2
    (void)mode;
    return err;
#else
    if (mode != 0 && !g_graphics_allowed) {
        err.code = 5;
        err.message = "Graphics not enabled. Launch with --sdl or --sdl-ondemand.";
        return err;
    }

    graphics_mode = mode;
    if (mode == 0) {
        /* Close graphics window and return to text mode */
        VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
        if (con && con_ops_saved) {
            con->ops = original_con_ops;
        }
        if (sdl_texture) { SDL_DestroyTexture(sdl_texture); sdl_texture = NULL; }
        if (sdl_renderer) { SDL_DestroyRenderer(sdl_renderer); sdl_renderer = NULL; }
        if (sdl_window) { SDL_DestroyWindow(sdl_window); sdl_window = NULL; }
        if (g_pixels) { free(g_pixels); g_pixels = NULL; }
        bgi_shutdown_mode(); /* Shutdown BGI alongside SDL2 */
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return err;
    }

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        err.code = 5; err.message = "Failed to initialize SDL Video subsystem";
        return err;
    }

    init_palette();

    g_width = 640;
    g_height = 480;
    g_grid_cols = 80;
    g_grid_rows = 30;

    if (mode == 13) {
        g_width = 320;
        g_height = 200;
        g_grid_cols = 40;
        g_grid_rows = 25;
    }

    if (sdl_texture) { SDL_DestroyTexture(sdl_texture); sdl_texture = NULL; }
    if (sdl_renderer) { SDL_DestroyRenderer(sdl_renderer); sdl_renderer = NULL; }
    if (sdl_window) { SDL_DestroyWindow(sdl_window); sdl_window = NULL; }
    if (g_pixels) { free(g_pixels); g_pixels = NULL; }

    sdl_window = SDL_CreateWindow("BASIC++ Graphics Screen",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  g_width, g_height, SDL_WINDOW_SHOWN);
    if (!sdl_window) {
        err.code = 5; err.message = "Failed to create SDL window";
        return err;
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdl_renderer) {
        sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!sdl_renderer) {
        err.code = 5; err.message = "Failed to create SDL renderer";
        return err;
    }

    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888, 
                                    SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    if (!sdl_texture) {
        err.code = 5; err.message = "Failed to create SDL texture";
        return err;
    }

    g_pixels = (uint32_t *)calloc(g_width * g_height, sizeof(uint32_t));
    if (!g_pixels) {
        err.code = 5; err.message = "Failed to allocate pixel buffer";
        return err;
    }

    g_cursor_x = 0;
    g_cursor_y = 0;

    /* Override CON: device ops */
    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) {
        if (!con_ops_saved) {
            original_con_ops = con->ops;
            con_ops_saved = true;
        }
        con->ops.putc = gfx_con_putc;
        con->ops.puts = gfx_con_puts;
        con->ops.cls  = gfx_con_cls;
        con->ops.flush = gfx_con_flush;
        con->ops.getc = gfx_con_getc;
        con->ops.gets = gfx_con_gets;
    }

    SDL_StartTextInput();
    gfx_con_cls(con);

    /* Initialize BGI for this SCREEN mode (BGI = primary renderer) */
    bgi_sync_screen_mode(mode);

    return err;
#endif
}

/**
 * @brief COLOR statement handler.
 * Syntax: COLOR foreground, background
 */
BppError stmt_color_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex); /* Consume '=' */
    }

    BValue fg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int fg = (int)fg_val.as.number;

    bool has_bg = false;
    int bg = 0;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue bg_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        bg = (int)bg_val.as.number;
        has_bg = true;
    }

    if (fg >= 0 && fg < 256) fg_color_idx = fg;
    if (has_bg && bg >= 0 && bg < 256) bg_color_idx = bg;

    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        static const int ansi_fg_map[16] = {30,34,32,36,31,35,33,37,90,94,92,96,91,95,93,97};
        static const int ansi_bg_map[16] = {40,44,42,46,41,45,43,47,100,104,102,106,101,105,103,107};
        char esc[64] = "";
        int fg_ansi = (fg >= 0 && fg < 16) ? ansi_fg_map[fg] : fg;
        int bg_ansi = (bg >= 0 && bg < 16) ? ansi_bg_map[bg] : bg;
        
        if (has_bg) {
            if (fg < 16 && bg < 16) {
                snprintf(esc, sizeof(esc), "\033[%d;%dm", fg_ansi, bg_ansi);
            } else if (fg >= 16 && bg >= 16) {
                snprintf(esc, sizeof(esc), "\033[38;5;%d;48;5;%dm", fg, bg);
            } else if (fg >= 16) {
                snprintf(esc, sizeof(esc), "\033[38;5;%d;%dm", fg, bg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[%d;48;5;%dm", fg_ansi, bg);
            }
        } else {
            if (fg < 16) {
                snprintf(esc, sizeof(esc), "\033[%dm", fg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[38;5;%dm", fg);
            }
        }
        vdev_puts(vdev_ctx, esc);
    }

    return err;
}

/**
 * @brief LINE statement handler.
 * Syntax: LINE (x1, y1)-(x2, y2), [color], [B | BF]
 */
BppError stmt_line_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) {
        lex_next(lex); /* Consume INPUT */
        return stmt_line_input_handler(vm, lex);
    }

    tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in LINE coordinates";
        return err;
    }
    BValue x1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in LINE coordinates";
        return err;
    }
    BValue y1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in LINE coordinates";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_MINUS) {
        err.code = 2; err.message = "Expected '-' in LINE statement";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in LINE coordinates";
        return err;
    }
    BValue x2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in LINE coordinates";
        return err;
    }
    BValue y2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in LINE coordinates";
        return err;
    }

    int color = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_EOL && tok.type != TOK_EOF) {
            BValue col_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            color = (int)col_val.as.number;
        }
    }

    bool box = false;
    bool box_fill = false;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type == TOK_IDENT) {
            char flag[16] = {0};
            size_t flen = (tok.length < 15) ? tok.length : 15;
            memcpy(flag, tok.start, flen);
            for (size_t i = 0; i < flen; i++) flag[i] = (char)toupper((unsigned char)flag[i]);

            if (strcmp(flag, "B") == 0) {
                box = true;
            } else if (strcmp(flag, "BF") == 0) {
                box = true;
                box_fill = true;
            }
        }
    }

#ifdef NO_SDL2
    (void)x1_val; (void)y1_val; (void)x2_val; (void)y2_val; (void)color; (void)box; (void)box_fill;
    return err;
#else
    if (!g_pixels) return err;

    int c_idx = (color >= 0 && color < 256) ? color : fg_color_idx;
    int x1 = (int)x1_val.as.number;
    int y1 = (int)y1_val.as.number;
    int x2 = (int)x2_val.as.number;
    int y2 = (int)y2_val.as.number;

    /* BGI primary renderer — try BGI first, fallback to direct g_pixels */
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, c_idx);
        if (box) {
            if (box_fill) {
                BGI_setfillstyle(bgi, BGI_SOLID_FILL, c_idx);
                BGI_bar(bgi, x1, y1, x2, y2);
            } else {
                BGI_rectangle(bgi, x1, y1, x2, y2);
            }
        } else {
            BGI_line(bgi, x1, y1, x2, y2);
        }
        bgi_sync_to_sdl();
    } else {
        /* Fallback: direct SDL g_pixels */
        uint32_t col = (uint32_t)((palette[c_idx].r << 24) |
                      (palette[c_idx].g << 16) |
                      (palette[c_idx].b << 8) | 255);
        if (box) {
            int start_y = (y1 < y2) ? y1 : y2;
            int end_y = (y1 < y2) ? y2 : y1;
            int start_x = (x1 < x2) ? x1 : x2;
            int end_x = (x1 < x2) ? x2 : x1;

            if (box_fill) {
                for (int y = start_y; y <= end_y; y++) {
                    if (y < 0 || y >= g_height) continue;
                    for (int x = start_x; x <= end_x; x++) {
                        if (x < 0 || x >= g_width) continue;
                        g_pixels[y * g_width + x] = col;
                    }
                }
            } else {
                draw_line(x1, y1, x2, y1, col);
                draw_line(x2, y1, x2, y2, col);
                draw_line(x2, y2, x1, y2, col);
                draw_line(x1, y2, x1, y1, col);
            }
        } else {
            draw_line(x1, y1, x2, y2, col);
        }
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
#endif
}

/**
 * @brief CIRCLE statement handler.
 * Syntax: CIRCLE (cx, cy), radius, [color]
 */
BppError stmt_circle_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in CIRCLE coordinates";
        return err;
    }
    BValue cx_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in CIRCLE coordinates";
        return err;
    }
    BValue cy_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in CIRCLE coordinates";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in CIRCLE radius";
        return err;
    }
    BValue rad_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int color = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue col_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        color = (int)col_val.as.number;
    }

#ifdef NO_SDL2
    (void)cx_val; (void)cy_val; (void)rad_val; (void)color;
    return err;
#else
    if (!g_pixels) return err;

    int c_idx = (color >= 0 && color < 256) ? color : fg_color_idx;
    int cx = (int)cx_val.as.number;
    int cy = (int)cy_val.as.number;
    int r = (int)rad_val.as.number;

    /* BGI primary renderer — try BGI first, fallback to direct draw_circle */
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, c_idx);
        BGI_circle(bgi, cx, cy, r);
        bgi_sync_to_sdl();
    } else {
        /* Fallback: direct SDL g_pixels */
        uint32_t col = (uint32_t)((palette[c_idx].r << 24) |
                      (palette[c_idx].g << 16) |
                      (palette[c_idx].b << 8) | 255);
        draw_circle(cx, cy, r, col);
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
#endif
}

/**
 * @brief PSET statement handler.
 * Syntax: PSET (x, y), [color]
 */
BppError stmt_pset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in PSET";
        return err;
    }
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in PSET";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in PSET";
        return err;
    }

    int color = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue col_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        color = (int)col_val.as.number;
    }

#ifdef NO_SDL2
    (void)x_val; (void)y_val; (void)color;
    return err;
#else
    if (!g_pixels) return err;

    int c_idx = (color >= 0 && color < 256) ? color : fg_color_idx;
    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;

    /* BGI primary renderer — try BGI first, fallback to direct g_pixels */
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi->initialized && bgi->framebuffer) {
        BGI_putpixel(bgi, x, y, c_idx);
        bgi_sync_to_sdl();
    } else {
        /* Fallback: direct SDL g_pixels write */
        uint32_t col = (uint32_t)((palette[c_idx].r << 24) |
                      (palette[c_idx].g << 16) |
                      (palette[c_idx].b << 8) | 255);
        if (x >= 0 && x < g_width && y >= 0 && y < g_height) {
            g_pixels[y * g_width + x] = col;
        }
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
#endif
}

/**
 * @brief PRESET statement handler.
 * Syntax: PRESET (x, y)
 */
BppError stmt_preset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in PRESET";
        return err;
    }
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in PRESET";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in PRESET";
        return err;
    }

#ifdef NO_SDL2
    (void)x_val; (void)y_val;
    return err;
#else
    if (!g_pixels) return err;

    uint32_t col = (palette[bg_color_idx].r << 24) | 
                  (palette[bg_color_idx].g << 16) | 
                  (palette[bg_color_idx].b << 8) | 255;

    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;

    if (x >= 0 && x < g_width && y >= 0 && y < g_height) {
        g_pixels[y * g_width + x] = col;
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
#endif
}

/**
 * @brief CLS statement handler.
 */
BppError stmt_cls_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

#ifdef NO_SDL2
    printf("\033[2J\033[H");
    fflush(stdout);
    return err;
#else
    if (!g_pixels) {
        printf("\033[2J\033[H");
        fflush(stdout);
        return err;
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_cls(con);

    return err;
#endif
}

/**
 * @brief PAINT statement handler.
 */
BppError stmt_paint_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' in PAINT";
        return err;
    }
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in PAINT";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' in PAINT";
        return err;
    }

    int paint_color = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue col_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        paint_color = (int)col_val.as.number;
    }

    int border_color = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue col_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        border_color = (int)col_val.as.number;
    }

#ifdef NO_SDL2
    (void)x_val; (void)y_val; (void)paint_color; (void)border_color;
    return err;
#else
    if (!g_pixels) return err;

    /* BGI primary renderer — try BGI first */
    BGI_Context *bgi_ctx = BGI_get_global_context();
    if (bgi_ctx->initialized && bgi_ctx->framebuffer) {
        int fill_idx = (paint_color >= 0 && paint_color < 256)
                       ? paint_color : fg_color_idx;
        int bord_idx = (border_color >= 0 && border_color < 256)
                       ? border_color : fill_idx;
        BGI_setfillstyle(bgi_ctx, BGI_SOLID_FILL, fill_idx);
        BGI_setcolor(bgi_ctx, bord_idx);
        BGI_floodfill(bgi_ctx, (int)x_val.as.number, (int)y_val.as.number,
                      bord_idx);
        bgi_sync_to_sdl();

        VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
        if (con) gfx_con_flush(con);
        return err;
    }

    /* Fallback: direct g_pixels flood fill */

    int sx = (int)x_val.as.number;
    int sy = (int)y_val.as.number;
    if (sx < 0 || sx >= g_width || sy < 0 || sy >= g_height) return err;

    /* Resolve fill color */
    int fill_idx = (paint_color >= 0 && paint_color < 256)
                   ? paint_color : fg_color_idx;
    uint32_t fill_argb = (uint32_t)((palette[fill_idx].r << 24) |
                         (palette[fill_idx].g << 16) |
                         (palette[fill_idx].b << 8) | 255);

    /* Resolve border color */
    uint32_t border_argb;
    if (border_color >= 0 && border_color < 256) {
        border_argb = (uint32_t)((palette[border_color].r << 24) |
                     (palette[border_color].g << 16) |
                     (palette[border_color].b << 8) | 255);
    } else {
        border_argb = fill_argb; /* default: stop at fill color itself */
    }

    /* Get seed color and bail if already filled or on border */
    uint32_t seed_color = g_pixels[sy * g_width + sx];
    if (seed_color == border_argb || seed_color == fill_argb) return err;

    /* Iterative scanline span-fill (never recursive) */
    typedef struct { int x1; int x2; int y; int dir; } PaintSpan;
    size_t capacity = 256;
    size_t count = 0;
    PaintSpan *stack = (PaintSpan *)calloc(capacity, sizeof(PaintSpan));
    if (!stack) { err.code = 14; err.message = "Out of memory in PAINT"; return err; }

    /* Push initial spans in both directions */
    stack[count].x1 = sx; stack[count].x2 = sx; stack[count].y = sy; stack[count].dir = 1; count++;
    stack[count].x1 = sx; stack[count].x2 = sx; stack[count].y = sy - 1; stack[count].dir = -1; count++;

    while (count > 0) {
        count--;
        PaintSpan span = stack[count];
        int cy = span.y;
        if (cy < 0 || cy >= g_height) continue;

        /* Find left extent */
        int lx = span.x1;
        while (lx > 0) {
            uint32_t c = g_pixels[cy * g_width + (lx - 1)];
            if (c == border_argb || c == fill_argb) break;
            lx--;
        }

        /* Find right extent */
        int rx = span.x2;
        while (rx < g_width - 1) {
            uint32_t c = g_pixels[cy * g_width + (rx + 1)];
            if (c == border_argb || c == fill_argb) break;
            rx++;
        }

        /* Fill the span */
        for (int fx = lx; fx <= rx; ++fx) {
            g_pixels[cy * g_width + fx] = fill_argb;
        }

        /* Scan above and below for new spans to push */
        int dirs[2] = { 1, -1 };
        for (int d = 0; d < 2; ++d) {
            int ny = cy + dirs[d];
            if (ny < 0 || ny >= g_height) continue;

            int scan_x = lx;
            while (scan_x <= rx) {
                uint32_t c = g_pixels[ny * g_width + scan_x];
                if (c == border_argb || c == fill_argb) {
                    scan_x++;
                    continue;
                }
                int span_start = scan_x;
                while (scan_x <= rx) {
                    c = g_pixels[ny * g_width + scan_x];
                    if (c == border_argb || c == fill_argb) break;
                    scan_x++;
                }
                /* Grow stack if needed (safe realloc pattern) */
                if (count >= capacity) {
                    size_t new_cap = capacity * 2;
                    PaintSpan *new_stack = (PaintSpan *)calloc(new_cap, sizeof(PaintSpan));
                    if (!new_stack) { free(stack); err.code = 14; err.message = "Out of memory in PAINT"; return err; }
                    memcpy(new_stack, stack, count * sizeof(PaintSpan));
                    free(stack);
                    stack = new_stack;
                    capacity = new_cap;
                }
                stack[count].x1 = span_start;
                stack[count].x2 = scan_x - 1;
                stack[count].y = ny;
                stack[count].dir = dirs[d];
                count++;
            }
        }
    }
    free(stack);

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
#endif
}

#ifndef NO_SDL2
#include "security/security.h"
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Tone generation state */
typedef struct {
    double frequency;
    double phase;
    int samples_remaining;
    int sample_rate;
} ToneState;

typedef struct {
    double frequency;
    double duration_seconds;
} MusicNote;

#define MAX_MUSIC_QUEUE 256
static ToneState g_tone_state = {0.0, 0.0, 0, 44100};
static MusicNote g_music_queue[MAX_MUSIC_QUEUE];
static int g_queue_head = 0;
static int g_queue_tail = 0;
static int g_queue_size = 0;
static SDL_AudioDeviceID g_audio_device = 0;
static bool g_audio_initialized = false;

/* Mouse Event queue moved to top of file */

/* Joystick state */
static SDL_Joystick *g_joystick1 = NULL;
static SDL_Joystick *g_joystick2 = NULL;

/* Software sound channels */
typedef struct {
    float *samples;
    uint32_t total_samples;
    uint32_t current_sample;
    float volume;
    bool active;
    bool loop;
    bool paused;
} SoundChannel;

#define MAX_SOUND_CHANNELS 32
static SoundChannel g_sound_channels[MAX_SOUND_CHANNELS];

/* Noise generator state */
typedef struct {
    int type; /* 0=none, 1=white, 2=pink, 3=brown */
    int samples_remaining;
    float last_value;
    float b0, b1, b2, b3, b4, b5, b6;
} NoiseState;
static NoiseState g_noise_state = {0, 0, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

/* Software image slots */
typedef struct {
    uint32_t *pixels;
    int width;
    int height;
    bool active;
} ImageSlot;

#define MAX_IMAGE_SLOTS 256
static ImageSlot g_image_slots[MAX_IMAGE_SLOTS];

/* Window resizing */
static bool g_window_resizable = false;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    ToneState *state = (ToneState *)userdata;
    float *buffer = (float *)stream;
    int num_samples = len / sizeof(float);

    for (int i = 0; i < num_samples; ++i) {
        float mixed = 0.0f;

        /* 1. Tone Generator */
        if (state->samples_remaining <= 0) {
            if (g_queue_size > 0) {
                MusicNote note = g_music_queue[g_queue_head];
                g_queue_head = (g_queue_head + 1) % MAX_MUSIC_QUEUE;
                g_queue_size--;
                state->frequency = note.frequency;
                state->samples_remaining = (int)(note.duration_seconds * state->sample_rate);
                state->phase = 0.0;
            } else {
                state->frequency = 0.0;
                state->phase = 0.0;
            }
        }

        if (state->samples_remaining > 0) {
            if (state->frequency > 0.0) {
                mixed = (float)(0.08 * sin(state->phase));
                state->phase += 2.0 * M_PI * state->frequency / state->sample_rate;
                if (state->phase >= 2.0 * M_PI) {
                    state->phase -= 2.0 * M_PI;
                }
            }
            state->samples_remaining--;
        }

        /* 2. Noise Generator */
        if (g_noise_state.samples_remaining > 0) {
            float n = 0.0f;
            if (g_noise_state.type == 1) {
                n = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
            } else if (g_noise_state.type == 2) {
                float white = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
                g_noise_state.b0 = 0.99886f * g_noise_state.b0 + white * 0.0555179f;
                g_noise_state.b1 = 0.99332f * g_noise_state.b1 + white * 0.0750759f;
                g_noise_state.b2 = 0.96900f * g_noise_state.b2 + white * 0.1538520f;
                g_noise_state.b3 = 0.86650f * g_noise_state.b3 + white * 0.3104856f;
                g_noise_state.b4 = 0.55000f * g_noise_state.b4 + white * 0.5329522f;
                g_noise_state.b5 = -0.7616f * g_noise_state.b5 - white * 0.0168980f;
                float pink = g_noise_state.b0 + g_noise_state.b1 + g_noise_state.b2 + g_noise_state.b3 + g_noise_state.b4 + g_noise_state.b5 + g_noise_state.b6 + white * 0.5362f;
                g_noise_state.b6 = white * 0.115926f;
                n = pink * 0.11f;
            } else if (g_noise_state.type == 3) {
                float white = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
                float val = g_noise_state.last_value + (white * 0.1f);
                if (val > 1.0f) val = 1.0f;
                else if (val < -1.0f) val = -1.0f;
                g_noise_state.last_value = val;
                n = val;
            }
            mixed += n * 0.08f;
            g_noise_state.samples_remaining--;
        }

        /* 3. software mixed playing channels */
        for (int ch = 0; ch < MAX_SOUND_CHANNELS; ch++) {
            SoundChannel *channel = &g_sound_channels[ch];
            if (channel->active && !channel->paused && channel->samples) {
                float sample = channel->samples[channel->current_sample];
                mixed += sample * channel->volume;
                channel->current_sample++;
                if (channel->current_sample >= channel->total_samples) {
                    if (channel->loop) {
                        channel->current_sample = 0;
                    } else {
                        channel->active = false;
                    }
                }
            }
        }

        if (mixed > 1.0f) mixed = 1.0f;
        else if (mixed < -1.0f) mixed = -1.0f;

        buffer[i] = mixed;
    }
}

static bool init_audio(void) {
    if (g_audio_initialized) return true;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        return false;
    }

    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = AUDIO_F32SYS;
    desired.channels = 1; /* Mono */
    desired.samples = 2048;
    desired.callback = audio_callback;
    desired.userdata = &g_tone_state;

    g_audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (g_audio_device == 0) {
        return false;
    }

    g_tone_state.sample_rate = obtained.freq;
    g_tone_state.frequency = 0.0;
    g_tone_state.phase = 0.0;
    g_tone_state.samples_remaining = 0;

    /* Start playing silence immediately (keeps the audio device running) */
    SDL_PauseAudioDevice(g_audio_device, 0);

    g_audio_initialized = true;
    return true;
}

static void push_note(double frequency, double duration_seconds, bool background) {
    if (!init_audio()) return;

    /* If queue is full, block/wait for space (or drop if we wait too long) */
    int max_retries = 100;
    while (true) {
        SDL_LockAudioDevice(g_audio_device);
        int size = g_queue_size;
        SDL_UnlockAudioDevice(g_audio_device);

        if (size < MAX_MUSIC_QUEUE) {
            break;
        }
        if (max_retries-- <= 0) {
            return; /* Drop note if queue remains full */
        }
        SDL_Delay(10);
    }

    SDL_LockAudioDevice(g_audio_device);
    MusicNote note;
    note.frequency = frequency;
    note.duration_seconds = duration_seconds;
    g_music_queue[g_queue_tail] = note;
    g_queue_tail = (g_queue_tail + 1) % MAX_MUSIC_QUEUE;
    g_queue_size++;
    SDL_UnlockAudioDevice(g_audio_device);

    /* If foreground, wait until the queue is fully drained */
    if (!background) {
        while (true) {
            SDL_LockAudioDevice(g_audio_device);
            int size = g_queue_size;
            int remaining_samples = g_tone_state.samples_remaining;
            SDL_UnlockAudioDevice(g_audio_device);

            if (size == 0 && remaining_samples == 0) {
                break;
            }
            SDL_Delay(10);
        }
    }
}

int vdev_music_note_count(void) {
#ifndef NO_SDL2
    if (!g_audio_initialized) return 0;
    SDL_LockAudioDevice(g_audio_device);
    int size = g_queue_size;
    SDL_UnlockAudioDevice(g_audio_device);
    return size;
#else
    return 0;
#endif
}

void vdev_gfx_beep(VDevContext *ctx) {
    (void)ctx;
    if (g_graphics_allowed && init_audio()) {
        push_note(800.0, 0.25, true); /* Play asynchronously in the background to prevent program stuttering */
    }
}

void vdev_play_sound_freq(double freq, double duration_seconds) {
    if (g_graphics_allowed && init_audio()) {
        push_note(freq, duration_seconds, true);
    }
}

int vdev_music_queue_length(void) {
    if (!g_audio_initialized || g_audio_device == 0) return 0;
    SDL_LockAudioDevice(g_audio_device);
    int size = g_queue_size;
    SDL_UnlockAudioDevice(g_audio_device);
    return size;
}

void vdev_music_clear(void) {
    if (!g_audio_initialized || g_audio_device == 0) return;
    SDL_LockAudioDevice(g_audio_device);
    g_queue_head = 0;
    g_queue_tail = 0;
    g_queue_size = 0;
    g_tone_state.frequency = 0.0;
    g_tone_state.phase = 0.0;
    g_tone_state.samples_remaining = 0;
    SDL_UnlockAudioDevice(g_audio_device);
}

void play_mml(VMContext *vm, const char *mml) {
    (void)vm;
    int tempo = 120;
    int octave = 4;
    int length = 4;
    bool background = true; /* default is background (blocks if MF is explicitly set) */

    const char *p = mml;
    while (*p) {
        /* Skip whitespace */
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        char cmd = (char)toupper((unsigned char)*p);
        p++;

        if (cmd == 'T') {
            int val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 32 && val <= 255) {
                tempo = val;
            }
        } else if (cmd == 'O') {
            int val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 0 && val <= 6) {
                octave = val;
            }
        } else if (cmd == 'L') {
            int val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val >= 1 && val <= 64) {
                length = val;
            }
        } else if (cmd == 'M') {
            if (*p) {
                char mode = (char)toupper((unsigned char)*p);
                p++;
                if (mode == 'B') background = true;
                else if (mode == 'F') background = false;
            }
        } else if (cmd == '<') {
            if (octave > 0) octave--;
        } else if (cmd == '>') {
            if (octave < 6) octave++;
        } else if (cmd >= 'A' && cmd <= 'G') {
            int note_offset = 0;
            switch (cmd) {
                case 'C': note_offset = 0; break;
                case 'D': note_offset = 2; break;
                case 'E': note_offset = 4; break;
                case 'F': note_offset = 5; break;
                case 'G': note_offset = 7; break;
                case 'A': note_offset = 9; break;
                case 'B': note_offset = 11; break;
            }
            if (*p == '+' || *p == '#') {
                note_offset++;
                p++;
            } else if (*p == '-') {
                note_offset--;
                p++;
            }

            int note_len = length;
            int len_val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                len_val = len_val * 10 + (*p - '0');
                p++;
            }
            if (len_val >= 1 && len_val <= 64) {
                note_len = len_val;
            }

            double dur_mult = 1.0;
            if (*p == '.') {
                dur_mult = 1.5;
                p++;
            }

            int midi = (octave + 1) * 12 + note_offset;
            double freq = 440.0 * pow(2.0, (midi - 69) / 12.0);
            double dur = (240.0 / (tempo * note_len)) * dur_mult;

            push_note(freq, dur, background);
        } else if (cmd == 'P') {
            int note_len = length;
            int len_val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                len_val = len_val * 10 + (*p - '0');
                p++;
            }
            if (len_val >= 1 && len_val <= 64) {
                note_len = len_val;
            }

            double dur_mult = 1.0;
            if (*p == '.') {
                dur_mult = 1.5;
                p++;
            }

            double dur = (240.0 / (tempo * note_len)) * dur_mult;
            push_note(0.0, dur, background);
        } else if (cmd == 'N') {
            int note_val = 0;
            while (*p && isdigit((unsigned char)*p)) {
                note_val = note_val * 10 + (*p - '0');
                p++;
            }
            if (note_val >= 0 && note_val <= 84) {
                if (note_val == 0) {
                    double dur = 240.0 / (tempo * length);
                    push_note(0.0, dur, background);
                } else {
                    int midi = 11 + note_val;
                    double freq = 440.0 * pow(2.0, (midi - 69) / 12.0);
                    double dur = 240.0 / (tempo * length);
                    push_note(freq, dur, background);
                }
            }
        }
    }
}
#endif

/**
 * @brief SOUND statement handler.
 */
BppError stmt_sound_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

#ifndef BPP_LITE_BUILD
    /* Security Check: requires virtual device access */
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: SOUND virtual device access blocked under sandbox settings";
        return err;
    }
#endif

    BValue freq_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in SOUND";
        return err;
    }

    BValue dur_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (freq_val.type == VAL_STRING || dur_val.type == VAL_STRING) {
        err.code = 13; err.message = "Type mismatch: SOUND expects numeric arguments";
        return err;
    }

    double freq = freq_val.as.number;
    double dur_ticks = dur_val.as.number;

    if (freq < 0.0 || freq > 32767.0) {
        err.code = 5; err.message = "Illegal function call: SOUND frequency must be 0 to 32767 Hz";
        return err;
    }
    if (dur_ticks < 0.0 || dur_ticks > 65535.0) {
        err.code = 5; err.message = "Illegal function call: SOUND duration must be 0 to 65535 ticks";
        return err;
    }

#ifndef NO_SDL2
    if (g_graphics_allowed) {
        double duration_sec = dur_ticks / 18.2;
        /* SOUND in QBASIC is asynchronous, meaning it plays in background. */
        push_note(freq, duration_sec, true);
    }
#endif

    return err;
}

/**
 * @brief PLAY statement handler.
 */
BppError stmt_play_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

#ifndef BPP_LITE_BUILD
    /* Security Check: requires virtual device access */
    if (security_check(SECOP_VDEV, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: PLAY virtual device access blocked under sandbox settings";
        return err;
    }
#endif

    BppToken peek = lex_peek(lex);
    if (peek.type == TOK_KEYWORD && (peek.as.keyword == KW_ON || peek.as.keyword == KW_OFF || peek.as.keyword == KW_STOP)) {
        lex_next(lex); /* Consume ON/OFF/STOP */
        if (peek.as.keyword == KW_ON) {
            vm_set_play_state(vm, 1);
        } else if (peek.as.keyword == KW_OFF) {
            vm_set_play_state(vm, 0);
        } else {
            vm_set_play_state(vm, 2);
        }
        return err;
    }

    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cmd_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch: PLAY expects command string";
        return err;
    }

    const char *mml_str = str_data(cmd_val.as.string);

#ifndef NO_SDL2
    if (g_graphics_allowed) {
        /* If empty string is passed, clear the music playback queue immediately */
        if (strlen(mml_str) == 0) {
            vdev_music_clear();
        } else {
            play_mml(vm, mml_str);
        }
    }
#endif

    str_release(vm_get_str(vm), cmd_val.as.string);
    return err;
}

static int g_last_plot_x = 0;
static int g_last_plot_y = 0;

static void gfx_pset(int x, int y, uint32_t col) {
    if (!g_pixels || x < 0 || x >= g_width || y < 0 || y >= g_height) return;
    g_pixels[y * g_width + x] = col;
}

BppError stmt_gr_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 40, 48);
    if (err.code == 0 && g_pixels) {
        memset(g_pixels, 0, g_width * g_height * sizeof(uint32_t));
    }
    return err;
}

BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 280, 192);
    if (err.code == 0 && g_pixels) {
        memset(g_pixels, 0, g_width * g_height * sizeof(uint32_t));
    }
    return err;
}

BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 280, 192);
    if (err.code == 0 && g_pixels) {
        memset(g_pixels, 0, g_width * g_height * sizeof(uint32_t));
    }
    return err;
}

BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex); /* Consume '=' */
    }
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "HCOLOR expects a numeric value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        fg_color_idx = color;
    }
    return err;
}

BppError stmt_plot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in PLOT";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (x_val.type == VAL_STRING || y_val.type == VAL_STRING) {
        err.code = 13; err.message = "PLOT coordinates must be numeric";
        return err;
    }
    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;
    g_last_plot_x = x;
    g_last_plot_y = y;
    if (g_pixels) {
        uint32_t col = (palette[fg_color_idx].r << 24) |
                      (palette[fg_color_idx].g << 16) |
                      (palette[fg_color_idx].b << 8) | 255;
        gfx_pset(x, y, col);
    }
    return err;
}

BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue x1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in HLIN";
        return err;
    }
    BValue x2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT || tok.length != 2 || strncasecmp(tok.start, "AT", 2) != 0) {
        err.code = 2; err.message = "Expected 'AT' in HLIN";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (x1_val.type == VAL_STRING || x2_val.type == VAL_STRING || y_val.type == VAL_STRING) {
        err.code = 13; err.message = "HLIN parameters must be numeric";
        return err;
    }
    int x1 = (int)x1_val.as.number;
    int x2 = (int)x2_val.as.number;
    int y = (int)y_val.as.number;
    if (g_pixels) {
        uint32_t col = (palette[fg_color_idx].r << 24) |
                      (palette[fg_color_idx].g << 16) |
                      (palette[fg_color_idx].b << 8) | 255;
        int start = (x1 < x2) ? x1 : x2;
        int end = (x1 < x2) ? x2 : x1;
        for (int x = start; x <= end; x++) {
            gfx_pset(x, y, col);
        }
    }
    return err;
}

BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue y1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in VLIN";
        return err;
    }
    BValue y2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT || tok.length != 2 || strncasecmp(tok.start, "AT", 2) != 0) {
        err.code = 2; err.message = "Expected 'AT' in VLIN";
        return err;
    }
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (y1_val.type == VAL_STRING || y2_val.type == VAL_STRING || x_val.type == VAL_STRING) {
        err.code = 13; err.message = "VLIN parameters must be numeric";
        return err;
    }
    int y1 = (int)y1_val.as.number;
    int y2 = (int)y2_val.as.number;
    int x = (int)x_val.as.number;
    if (g_pixels) {
        uint32_t col = (palette[fg_color_idx].r << 24) |
                      (palette[fg_color_idx].g << 16) |
                      (palette[fg_color_idx].b << 8) | 255;
        int start = (y1 < y2) ? y1 : y2;
        int end = (y1 < y2) ? y2 : y1;
        for (int y = start; y <= end; y++) {
            gfx_pset(x, y, col);
        }
    }
    return err;
}

BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    uint32_t col = 0xFFFFFFFF;
    if (g_pixels) {
        col = (palette[fg_color_idx].r << 24) |
              (palette[fg_color_idx].g << 16) |
              (palette[fg_color_idx].b << 8) | 255;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) {
        /* HPLOT TO x, y */
        lex_next(lex); /* Consume 'TO' */
        BValue x_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in HPLOT TO";
            return err;
        }
        BValue y_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (x_val.type == VAL_STRING || y_val.type == VAL_STRING) {
            err.code = 13; err.message = "HPLOT coordinates must be numeric";
            return err;
        }
        int x = (int)x_val.as.number;
        int y = (int)y_val.as.number;
#ifndef NO_SDL2
        if (g_pixels) {
            draw_line(g_last_plot_x, g_last_plot_y, x, y, col);
        }
#endif
        g_last_plot_x = x;
        g_last_plot_y = y;
    } else {
        /* HPLOT x, y [TO x2, y2 ...] */
        BValue x_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in HPLOT";
            return err;
        }
        BValue y_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (x_val.type == VAL_STRING || y_val.type == VAL_STRING) {
            err.code = 13; err.message = "HPLOT coordinates must be numeric";
            return err;
        }
        int x = (int)x_val.as.number;
        int y = (int)y_val.as.number;
        if (g_pixels) {
            gfx_pset(x, y, col);
        }
        g_last_plot_x = x;
        g_last_plot_y = y;

        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) {
                lex_next(lex); /* Consume 'TO' */
                BValue nx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                tok = lex_next(lex);
                if (tok.type != TOK_COMMA) {
                    err.code = 2; err.message = "Expected ',' in HPLOT TO chain";
                    return err;
                }
                BValue ny_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (nx_val.type == VAL_STRING || ny_val.type == VAL_STRING) {
                    err.code = 13; err.message = "HPLOT coordinates must be numeric";
                    return err;
                }
                int nx = (int)nx_val.as.number;
                int ny = (int)ny_val.as.number;
#ifndef NO_SDL2
                if (g_pixels) {
                    draw_line(g_last_plot_x, g_last_plot_y, nx, ny, col);
                }
#endif
                g_last_plot_x = nx;
                g_last_plot_y = ny;
            } else {
                break;
            }
        }
    }
    return err;
}

BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (mode_val.type == VAL_STRING) {
        err.code = 13; err.message = "GRAPHICS mode must be numeric";
        return err;
    }
    int mode = (int)mode_val.as.number;
    int w = 320, h = 192;
    if (mode == 0) { w = 320; h = 240; }
    else if (mode == 3) { w = 40; h = 24; }
    else if (mode == 5) { w = 80; h = 48; }
    else if (mode == 7) { w = 160; h = 96; }
    else if (mode == 8) { w = 320; h = 192; }

    err = init_graphics_mode_dims(vm, w, h);
    if (err.code == 0 && g_pixels) {
        memset(g_pixels, 0, g_width * g_height * sizeof(uint32_t));
    }
    return err;
}

BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' in DRAWTO";
        return err;
    }
    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (x_val.type == VAL_STRING || y_val.type == VAL_STRING) {
        err.code = 13; err.message = "DRAWTO coordinates must be numeric";
        return err;
    }
    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;
#ifndef NO_SDL2
    if (g_pixels) {
        uint32_t col = (palette[fg_color_idx].r << 24) |
                      (palette[fg_color_idx].g << 16) |
                      (palette[fg_color_idx].b << 8) | 255;
        draw_line(g_last_plot_x, g_last_plot_y, x, y, col);
    }
#endif
    g_last_plot_x = x;
    g_last_plot_y = y;
    return err;
}

BppError stmt_border_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "BORDER expects a numeric color value";
        return err;
    }
    (void)val;
    return err;
}

BppError stmt_ink_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "INK expects a numeric color value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        fg_color_idx = color;
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_fg_map[16] = {30,34,32,36,31,35,33,37,90,94,92,96,91,95,93,97};
            int fg_ansi = (color < 16) ? ansi_fg_map[color] : color;
            char esc[64];
            if (color < 16) {
                snprintf(esc, sizeof(esc), "\033[%dm", fg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[38;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}

BppError stmt_paper_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "PAPER expects a numeric color value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        bg_color_idx = color;
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_bg_map[16] = {40,44,42,46,41,45,43,47,100,104,102,106,101,105,103,107};
            int bg_ansi = (color < 16) ? ansi_bg_map[color] : color;
            char esc[64];
            if (color < 16) {
                snprintf(esc, sizeof(esc), "\033[%dm", bg_ansi);
            } else {
                snprintf(esc, sizeof(esc), "\033[48;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}

int platform_mouse_x(void) {
    return g_current_mouse_state.x;
}

int platform_mouse_y(void) {
    return g_current_mouse_state.y;
}

int platform_mouse_btn(void) {
    int flags = 0;
    if (g_current_mouse_state.left_button) flags |= 1;
    if (g_current_mouse_state.right_button) flags |= 2;
    if (g_current_mouse_state.middle_button) flags |= 4;
    return flags;
}

int platform_mouse_query(int n) {
    switch (n) {
        case 0: return sdl_window ? -1 : 0;
        case 1: return g_current_mouse_state.x;
        case 2: return g_current_mouse_state.y;
        case 3: return g_current_mouse_state.left_button ? -1 : 0;
        case 4: return g_current_mouse_state.right_button ? -1 : 0;
        case 5: return g_current_mouse_state.middle_button ? -1 : 0;
        case 6: return g_current_mouse_state.wheel;
        default: return 0;
    }
}

bool platform_mouse_input(void) {
#ifndef NO_SDL2
    vdev_gfx_poll_events();
    if (g_mouse_size > 0) {
        g_current_mouse_state = g_mouse_queue[g_mouse_head];
        g_mouse_head = (g_mouse_head + 1) % MAX_MOUSE_QUEUE;
        g_mouse_size--;
        return true;
    }
#endif
    return false;
}

int platform_mouse_wheel(void) {
    return g_current_mouse_state.wheel;
}

void platform_mouse_hide(void) {
#ifndef NO_SDL2
    SDL_ShowCursor(SDL_DISABLE);
#endif
}

void platform_mouse_show(void) {
#ifndef NO_SDL2
    SDL_ShowCursor(SDL_ENABLE);
#endif
}

int platform_joystick_count(void) {
#ifndef NO_SDL2
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) >= 0) {
        return SDL_NumJoysticks();
    }
#endif
    return 0;
}

const char *platform_joystick_name(int index) {
#ifndef NO_SDL2
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) >= 0) {
        const char *name = SDL_JoystickNameForIndex(index);
        return name ? name : "Unknown Joystick";
    }
#endif
    return "No Joystick";
}

double platform_joystick_axis(int stick_idx, int axis_idx) {
#ifndef NO_SDL2
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (stick_idx == 0) {
        if (!g_joystick1 && SDL_NumJoysticks() > 0) {
            g_joystick1 = SDL_JoystickOpen(0);
        }
        if (g_joystick1) {
            SDL_JoystickUpdate();
            int val = SDL_JoystickGetAxis(g_joystick1, axis_idx);
            return (double)val / 32768.0;
        }
    } else if (stick_idx == 1) {
        if (!g_joystick2 && SDL_NumJoysticks() > 1) {
            g_joystick2 = SDL_JoystickOpen(1);
        }
        if (g_joystick2) {
            SDL_JoystickUpdate();
            int val = SDL_JoystickGetAxis(g_joystick2, axis_idx);
            return (double)val / 32768.0;
        }
    }
#else
    (void)stick_idx; (void)axis_idx;
#endif
    return 0.0;
}

int platform_joystick_button(int stick_idx, int button_idx) {
#ifndef NO_SDL2
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (stick_idx == 0) {
        if (!g_joystick1 && SDL_NumJoysticks() > 0) {
            g_joystick1 = SDL_JoystickOpen(0);
        }
        if (g_joystick1) {
            SDL_JoystickUpdate();
            return SDL_JoystickGetButton(g_joystick1, button_idx);
        }
    } else if (stick_idx == 1) {
        if (!g_joystick2 && SDL_NumJoysticks() > 1) {
            g_joystick2 = SDL_JoystickOpen(1);
        }
        if (g_joystick2) {
            SDL_JoystickUpdate();
            return SDL_JoystickGetButton(g_joystick2, button_idx);
        }
    }
#else
    (void)stick_idx; (void)button_idx;
#endif
    return 0;
}

void platform_window_title_set(const char *title) {
#ifndef NO_SDL2
    if (sdl_window) {
        SDL_SetWindowTitle(sdl_window, title);
    }
#else
    (void)title;
#endif
}

void platform_window_move(int x, int y) {
#ifndef NO_SDL2
    if (sdl_window) {
        SDL_SetWindowPosition(sdl_window, x, y);
    }
#else
    (void)x; (void)y;
#endif
}

void platform_window_fullscreen(void) {
#ifndef NO_SDL2
    if (sdl_window) {
        uint32_t flags = SDL_GetWindowFlags(sdl_window);
        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            SDL_SetWindowFullscreen(sdl_window, 0);
        } else {
            SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
    }
#endif
}

int platform_window_x(void) {
#ifndef NO_SDL2
    int x = 0, y = 0;
    if (sdl_window) {
        SDL_GetWindowPosition(sdl_window, &x, &y);
    }
    return x;
#else
    return 0;
#endif
}

int platform_window_y(void) {
#ifndef NO_SDL2
    int x = 0, y = 0;
    if (sdl_window) {
        SDL_GetWindowPosition(sdl_window, &x, &y);
    }
    return y;
#else
    return 0;
#endif
}

int platform_desktop_width(void) {
#ifndef NO_SDL2
    SDL_DisplayMode mode;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) >= 0 && SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        return mode.w;
    }
#endif
    return 1920;
}

int platform_desktop_height(void) {
#ifndef NO_SDL2
    SDL_DisplayMode mode;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) >= 0 && SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        return mode.h;
    }
#endif
    return 1080;
}

void platform_window_resizable(bool resizable) {
    g_window_resizable = resizable;
#ifndef NO_SDL2
    if (sdl_window) {
        SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
    }
#endif
}

void platform_window_icon_set(const char *filename) {
#ifndef NO_SDL2
    if (sdl_window) {
        SDL_Surface *surf = SDL_LoadBMP(filename);
        if (surf) {
            SDL_SetWindowIcon(sdl_window, surf);
            SDL_FreeSurface(surf);
        }
    }
#else
    (void)filename;
#endif
}

int vdev_sound_open(const char *filename) {
    int slot = -1;
    for (int i = 0; i < MAX_SOUND_CHANNELS; i++) {
        if (!g_sound_channels[i].active && !g_sound_channels[i].samples) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

#ifndef NO_SDL2
    if (!g_audio_initialized) init_audio();

    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer = NULL;
    Uint32 wav_length = 0;
    if (SDL_LoadWAV(filename, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        return -1;
    }

    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          AUDIO_F32SYS, 1, 44100) < 0) {
        SDL_FreeWAV(wav_buffer);
        return -1;
    }

    cvt.len = wav_length;
    cvt.buf = (Uint8 *)calloc(1, cvt.len * cvt.len_mult);
    if (!cvt.buf) {
        SDL_FreeWAV(wav_buffer);
        return -1;
    }
    memcpy(cvt.buf, wav_buffer, wav_length);
    SDL_FreeWAV(wav_buffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        free(cvt.buf);
        return -1;
    }

    int num_samples = cvt.len_cvt / sizeof(float);
    float *samples = (float *)calloc(1, cvt.len_cvt);
    if (!samples) {
        free(cvt.buf);
        return -1;
    }
    memcpy(samples, cvt.buf, cvt.len_cvt);
    free(cvt.buf);

    g_sound_channels[slot].samples = samples;
    g_sound_channels[slot].total_samples = num_samples;
    g_sound_channels[slot].current_sample = 0;
    g_sound_channels[slot].volume = 1.0f;
    g_sound_channels[slot].active = false;
    g_sound_channels[slot].loop = false;
    g_sound_channels[slot].paused = false;

    return slot;
#else
    (void)filename;
    return -1;
#endif
}

void vdev_sound_play(int handle) {
#ifndef NO_SDL2
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS && g_sound_channels[handle].samples) {
        SDL_LockAudioDevice(g_audio_device);
        g_sound_channels[handle].active = true;
        g_sound_channels[handle].loop = false;
        g_sound_channels[handle].paused = false;
        g_sound_channels[handle].current_sample = 0;
        SDL_UnlockAudioDevice(g_audio_device);
    }
#else
    (void)handle;
#endif
}

void vdev_sound_loop(int handle) {
#ifndef NO_SDL2
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS && g_sound_channels[handle].samples) {
        SDL_LockAudioDevice(g_audio_device);
        g_sound_channels[handle].active = true;
        g_sound_channels[handle].loop = true;
        g_sound_channels[handle].paused = false;
        g_sound_channels[handle].current_sample = 0;
        SDL_UnlockAudioDevice(g_audio_device);
    }
#else
    (void)handle;
#endif
}

void vdev_sound_stop(int handle) {
#ifndef NO_SDL2
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS) {
        SDL_LockAudioDevice(g_audio_device);
        g_sound_channels[handle].active = false;
        g_sound_channels[handle].paused = false;
        SDL_UnlockAudioDevice(g_audio_device);
    }
#else
    (void)handle;
#endif
}

void vdev_sound_pause(int handle) {
#ifndef NO_SDL2
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS) {
        SDL_LockAudioDevice(g_audio_device);
        g_sound_channels[handle].paused = true;
        SDL_UnlockAudioDevice(g_audio_device);
    }
#else
    (void)handle;
#endif
}

void vdev_sound_volume(int handle, double vol) {
#ifndef NO_SDL2
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS) {
        SDL_LockAudioDevice(g_audio_device);
        g_sound_channels[handle].volume = (float)vol;
        SDL_UnlockAudioDevice(g_audio_device);
    }
#else
    (void)handle; (void)vol;
#endif
}

double vdev_sound_length(int handle) {
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS && g_sound_channels[handle].samples) {
        return (double)g_sound_channels[handle].total_samples / 44100.0;
    }
    return 0.0;
}

double vdev_sound_position(int handle) {
    if (handle >= 0 && handle < MAX_SOUND_CHANNELS && g_sound_channels[handle].samples) {
        return (double)g_sound_channels[handle].current_sample / 44100.0;
    }
    return 0.0;
}

void vdev_sound_noise(int type, double dur_seconds) {
#ifndef NO_SDL2
    if (!g_audio_initialized) init_audio();
    SDL_LockAudioDevice(g_audio_device);
    g_noise_state.type = type;
    g_noise_state.samples_remaining = (int)(dur_seconds * 44100.0);
    g_noise_state.last_value = 0.0f;
    SDL_UnlockAudioDevice(g_audio_device);
#else
    (void)type; (void)dur_seconds;
#endif
}

void vdev_sound_free_all(void) {
    for (int i = 0; i < MAX_SOUND_CHANNELS; i++) {
        if (g_sound_channels[i].samples) {
            free(g_sound_channels[i].samples);
            g_sound_channels[i].samples = NULL;
        }
        g_sound_channels[i].active = false;
        g_sound_channels[i].loop = false;
        g_sound_channels[i].paused = false;
        g_sound_channels[i].volume = 1.0f;
    }
#ifndef NO_SDL2
    if (g_joystick1) {
        SDL_JoystickClose(g_joystick1);
        g_joystick1 = NULL;
    }
    if (g_joystick2) {
        SDL_JoystickClose(g_joystick2);
        g_joystick2 = NULL;
    }
#endif
    g_noise_state.samples_remaining = 0;
    g_noise_state.type = 0;
}

int vdev_image_load(const char *filename) {
    int slot = -1;
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        if (!g_image_slots[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

#ifndef NO_SDL2
    SDL_Surface *surf = SDL_LoadBMP(filename);
    if (!surf) {
        return -1;
    }
    SDL_Surface *converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    if (!converted) {
        SDL_FreeSurface(surf);
        return -1;
    }

    int w = converted->w;
    int h = converted->h;
    uint32_t *pixels = (uint32_t *)calloc(w * h, sizeof(uint32_t));
    if (!pixels) {
        SDL_FreeSurface(converted);
        SDL_FreeSurface(surf);
        return -1;
    }

    memcpy(pixels, converted->pixels, w * h * sizeof(uint32_t));
    g_image_slots[slot].pixels = pixels;
    g_image_slots[slot].width = w;
    g_image_slots[slot].height = h;
    g_image_slots[slot].active = true;

    SDL_FreeSurface(converted);
    SDL_FreeSurface(surf);
    return slot;
#else
    (void)filename;
    return -1;
#endif
}

int vdev_image_create(int w, int h, int mode) {
    (void)mode;
    int slot = -1;
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        if (!g_image_slots[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    uint32_t *pixels = (uint32_t *)calloc(w * h, sizeof(uint32_t));
    if (!pixels) return -1;

    g_image_slots[slot].pixels = pixels;
    g_image_slots[slot].width = w;
    g_image_slots[slot].height = h;
    g_image_slots[slot].active = true;
    return slot;
}

void vdev_image_free(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        if (g_image_slots[handle].pixels) {
            free(g_image_slots[handle].pixels);
            g_image_slots[handle].pixels = NULL;
        }
        g_image_slots[handle].active = false;
    }
}

int vdev_image_copy(int handle) {
    if (handle < 0 || handle >= MAX_IMAGE_SLOTS || !g_image_slots[handle].active) return -1;
    int slot = -1;
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        if (!g_image_slots[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1;

    int w = g_image_slots[handle].width;
    int h = g_image_slots[handle].height;
    uint32_t *pixels = (uint32_t *)calloc(w * h, sizeof(uint32_t));
    if (!pixels) return -1;

    memcpy(pixels, g_image_slots[handle].pixels, w * h * sizeof(uint32_t));
    g_image_slots[slot].pixels = pixels;
    g_image_slots[slot].width = w;
    g_image_slots[slot].height = h;
    g_image_slots[slot].active = true;
    return slot;
}

void vdev_image_draw(int handle, int x, int y) {
    if (handle < 0 || handle >= MAX_IMAGE_SLOTS || !g_image_slots[handle].active) return;
    int w = g_image_slots[handle].width;
    int h = g_image_slots[handle].height;
    uint32_t *src = g_image_slots[handle].pixels;

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        for (int dy = 0; dy < h; dy++) {
            int py = y + dy;
            if (py < 0 || py >= g_height) continue;
            for (int dx = 0; dx < w; dx++) {
                int px = x + dx;
                if (px < 0 || px >= g_width) continue;
                bgi->framebuffer[py * g_width + px] = src[dy * w + dx];
            }
        }
        bgi_sync_to_sdl();
    } else if (g_pixels) {
        for (int dy = 0; dy < h; dy++) {
            int py = y + dy;
            if (py < 0 || py >= g_height) continue;
            for (int dx = 0; dx < w; dx++) {
                int px = x + dx;
                if (px < 0 || px >= g_width) continue;
                g_pixels[py * g_width + px] = src[dy * w + dx];
            }
        }
    }
}

int vdev_image_width(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        return g_image_slots[handle].width;
    }
    return 0;
}

int vdev_image_height(int handle) {
    if (handle >= 0 && handle < MAX_IMAGE_SLOTS && g_image_slots[handle].active) {
        return g_image_slots[handle].height;
    }
    return 0;
}

void vdev_image_free_all(void) {
    for (int i = 0; i < MAX_IMAGE_SLOTS; i++) {
        vdev_image_free(i);
    }
}

int platform_inkey_char(void) {
#ifndef NO_SDL2
    if (sdl_window) {
        vdev_gfx_poll_events();
        if (g_key_head != g_key_tail) {
            int code = g_key_buffer[g_key_head];
            g_key_head = (g_key_head + 1) % 64;
            return code;
        }
        return 0;
    }
#endif
    if (platform_kbhit()) {
        return platform_getch();
    }
    return 0;
}

int gfx_get_char_at(int row, int col) {
    if (row < 0 || row >= MAX_GRID_ROWS || col < 0 || col >= MAX_GRID_COLS) return 32;
    return (unsigned char)g_screen_chars[row][col];
}

int gfx_get_attr_at(int row, int col) {
    if (row < 0 || row >= MAX_GRID_ROWS || col < 0 || col >= MAX_GRID_COLS) return 7;
    return g_screen_attribs[row][col];
}



