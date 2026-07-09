/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: gfxbuf.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Screen mode setting, line/circle drawing algorithms, palette mappings, and ANSI/Unicode text-mode framebuffer.
 *
 * 2. WHAT TO EXPECT:
 *    Renders coordinate-scaled virtual buffer to terminal using Unicode half-blocks. High execution density.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default palette colors, screen dimensions limits, drawing layout algorithms.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Framebuffer structure, Unicode screen printing characters mappings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If display is garbled, verify terminal supports UTF-8 and ANSI escape codes. Check coordinate math ranges.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - gfxbuf.c
 // ---
 //
 // Virtual Graphics Framebuffer implementation.
 //
 // RENDERING STRATEGY:
 // The framebuffer stores color indices (0-15) per pixel in a
 // flat byte array. When gfxbuf_render() is called, it outputs
 // ANSI escape sequences with Unicode half-block characters.
 //
 // Each character cell displays 2 vertical pixels:
 // Upper half = pixel at (x, y)
 // Lower half = pixel at (x, y+1)
 //
 // Half-block char: U+2580 (upper half block)
 // The foreground color = upper pixel, background = lower pixel.
 // If both are the same, a full block or space is used.
 //
 // CGA-COMPATIBLE PALETTE:
 // Colors 0-15 map to standard ANSI terminal colors.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <stdio.h>
#include <string.h>
#include "gfxbuf.h"
#include "memory.h"
#include "segmented_mem.h"
#include "sdl2_emu.h"
#include "../console.h"

static int gfx_cursor_x = 0;
static int gfx_cursor_y = 0;

extern struct GW_Memory *g_gw_mem;

// Multi-page pixel buffers: pointer mapped to dynamic graphics memory pool
static unsigned char (*framepages)[GFX_WIDTH * GFX_HEIGHT] = NULL;

// Active drawing page and visual display page
static int active_page = 0;
static int visual_page = 0;

// Convenience pointer to active page
#define framebuf (framepages[active_page])

// Palette: maps color indices to ANSI color codes
static int palette[GFX_MAX_COLORS];

// Graphics mode active flag
static int gfx_active = 0;

// Viewport clipping rectangle (pixels)
static int vp_x1 = 0, vp_y1 = 0;
static int vp_x2 = GFX_WIDTH - 1;
static int vp_y2 = GFX_HEIGHT - 1;

// Default CGA palette -> ANSI 256-color indices
static const int default_palette[16] = {
 0, // 0: Black
 4, // 1: Blue
 2, // 2: Green
 6, // 3: Cyan
 1, // 4: Red
 5, // 5: Magenta
 3, // 6: Brown
 7, // 7: Light Gray
 8, // 8: Dark Gray
 12, // 9: Light Blue
 10, // 10: Light Green
 14, // 11: Light Cyan
 9, // 12: Light Red
 13, // 13: Light Magenta
 11, // 14: Yellow
 15 // 15: White
};

void gfxbuf_init_pool(MemorySystem *memory)
{
    int i;
    long required_size = (long)(GFX_MAX_PAGES * GFX_WIDTH * GFX_HEIGHT);
    if (memory && memory->graphics.base && memory->graphics.size >= required_size) {
        framepages = (unsigned char (*)[GFX_WIDTH * GFX_HEIGHT])memory->graphics.base;
    } else {
        static unsigned char fallback_pages[GFX_MAX_PAGES][GFX_WIDTH * GFX_HEIGHT];
        framepages = fallback_pages;
    }

    memset(framepages, 0, GFX_MAX_PAGES * GFX_WIDTH * GFX_HEIGHT);
    active_page = 0;
    visual_page = 0;
    for (i = 0; i < GFX_MAX_COLORS; i++) {
        palette[i] = default_palette[i];
    }
    gfx_active = 0;
    vp_x1 = 0; vp_y1 = 0;
    vp_x2 = GFX_WIDTH - 1;
    vp_y2 = GFX_HEIGHT - 1;
}

void gfxbuf_init(void)
{
    gfxbuf_init_pool(NULL);
}

void gfxbuf_clear(int color)
{
 if (color < 0) color = 0;
 if (color >= GFX_MAX_COLORS) color = 0;
 // When SDL is active, clear the SDL pixel buffer using the palette color
 if (gw_sdl2_is_active()) {
     uint32_t argb = GW_PALETTE[color % 256];
     (void)argb;
     gw_sdl2_clear(argb);
     gw_sdl2_present_force();
 }
 memset(framebuf, (unsigned char)color, GFX_WIDTH * GFX_HEIGHT);
}

void gfxbuf_pset(int x, int y, int color)
{
    gfxbuf_set_cursor(x, y);
#ifndef NO_SDL2
    if (!gw_sdl2_is_active()) {
        gw_sdl2_init(640, 400, "BASIC++ Emulation", 0);
        gw_sdl2_set_mode(0, 80);
    }
#endif
    if (gw_sdl2_is_active()) {
        int w = gw_sdl2_get_width();
        int h = gw_sdl2_get_height();
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        if (color < 0) color = 0;
        uint32_t argb = GW_PALETTE[color % 256];
        (void)argb;
        gw_sdl2_set_pixel(x, y, argb);
        return;
    }
 // Clip against viewport bounds
 if (x < vp_x1 || x > vp_x2 ||
 y < vp_y1 || y > vp_y2)
 return;
 if (x < 0 || x >= GFX_WIDTH ||
 y < 0 || y >= GFX_HEIGHT)
 return;
 if (color < 0) color = 0;
 if (color >= GFX_MAX_COLORS) color %= GFX_MAX_COLORS;
 framebuf[y * GFX_WIDTH + x] = (unsigned char)color;
}

int gfxbuf_point(int x, int y)
{
 if (gw_sdl2_is_active()) {
     int w = gw_sdl2_get_width();
     int h = gw_sdl2_get_height();
     if (x < 0 || x >= w || y < 0 || y >= h) return 0;
     uint32_t argb = gw_sdl2_get_pixel(x, y);
     for (int i = 0; i < 256; i++) {
         if (GW_PALETTE[i] == argb) return i;
     }
     return 0;
 }
 if (x < 0 || x >= GFX_WIDTH ||
 y < 0 || y >= GFX_HEIGHT)
 return 0;
 return (int)framepages[visual_page]
 [y * GFX_WIDTH + x];
}

 // Bresenham's line algorithm.
 // Draws a line from (x1,y1) to (x2,y2) in the given color.
void gfxbuf_line(int x1, int y1, int x2, int y2, int color)
{
    gfxbuf_set_cursor(x2, y2);
 int dx, dy, sx, sy, err, e2;

 dx = x2 - x1; if (dx < 0) dx = -dx;
 dy = y2 - y1; if (dy < 0) dy = -dy;
 sx = (x1 < x2) ? 1 : -1;
 sy = (y1 < y2) ? 1 : -1;
 err = dx - dy;

 for (;;) {
 gfxbuf_pset(x1, y1, color);
 if (x1 == x2 && y1 == y2) break;
 e2 = 2 * err;
 if (e2 > -dy) { err -= dy; x1 += sx; }
 if (e2 < dx) { err += dx; y1 += sy; }
 }
}

void gfxbuf_box(int x1, int y1, int x2, int y2,
 int color, int filled)
{
 int x, y, tmp;

 if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
 if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

 if (filled) {
 for (y = y1; y <= y2; y++)
 for (x = x1; x <= x2; x++)
 gfxbuf_pset(x, y, color);
 } else {
 for (x = x1; x <= x2; x++) {
 gfxbuf_pset(x, y1, color);
 gfxbuf_pset(x, y2, color);
 }
 for (y = y1; y <= y2; y++) {
 gfxbuf_pset(x1, y, color);
 gfxbuf_pset(x2, y, color);
 }
 }
}

 // Midpoint circle algorithm.
void gfxbuf_circle(int cx, int cy, int r, int color)
{
 int x = 0, y = r;
 int d = 1 - r;

 while (x <= y) {
 gfxbuf_pset(cx + x, cy + y, color);
 gfxbuf_pset(cx - x, cy + y, color);
 gfxbuf_pset(cx + x, cy - y, color);
 gfxbuf_pset(cx - x, cy - y, color);
 gfxbuf_pset(cx + y, cy + x, color);
 gfxbuf_pset(cx - y, cy + x, color);
 gfxbuf_pset(cx + y, cy - x, color);
 gfxbuf_pset(cx - y, cy - x, color);
 x++;
 if (d < 0) {
 d += 2 * x + 1;
 } else {
 y--;
 d += 2 * (x - y) + 1;
 }
 }
}

void gfxbuf_paint(MemorySystem *memory, int x, int y, int fill_color,
 int border_color)
{
    typedef struct { int x, y; } PaintPoint;
    PaintPoint *stack = NULL;
    int stack_capacity = 16384;
    int top = 0;
    int old_color;
    long old_scratch_used = 0;

    // Use dynamic pre-allocated Scratch Pool from the Canvas system if available
    if (memory && memory->scratch.base) {
        old_scratch_used = memory->scratch.used;
        long bytes_needed = (long)(stack_capacity * sizeof(PaintPoint));
        stack = (PaintPoint *)mem_pool_alloc(&memory->scratch, bytes_needed);
    }

    // Fallback queue on the CPU stack if no memory system is passed or pool is full
    PaintPoint fallback_stack[512];
    if (stack == NULL) {
        stack = fallback_stack;
        stack_capacity = 512;
    }

    // Use SDL dimensions when SDL is active, fallback to framebuffer dimensions
    int max_w = gw_sdl2_is_active() ? gw_sdl2_get_width() : GFX_WIDTH;
    int max_h = gw_sdl2_is_active() ? gw_sdl2_get_height() : GFX_HEIGHT;

    if (x < 0 || x >= max_w || y < 0 || y >= max_h) {
        if (memory && memory->scratch.base && stack != fallback_stack) {
            memory->scratch.used = old_scratch_used;
        }
        return;
    }

    old_color = gfxbuf_point(x, y);
    if (old_color == fill_color || old_color == border_color) {
        if (memory && memory->scratch.base && stack != fallback_stack) {
            memory->scratch.used = old_scratch_used;
        }
        return;
    }

    stack[top].x = x;
    stack[top].y = y;
    top++;

    while (top > 0) {
        int px, py, c;
        top--;
        px = stack[top].x;
        py = stack[top].y;

        if (px < 0 || px >= max_w || py < 0 || py >= max_h)
            continue;

        c = gfxbuf_point(px, py);
        if (c == fill_color || c == border_color)
            continue;

        gfxbuf_pset(px, py, fill_color);

        if (top + 4 < stack_capacity) {
            stack[top].x = px + 1; stack[top].y = py; top++;
            stack[top].x = px - 1; stack[top].y = py; top++;
            stack[top].x = px; stack[top].y = py + 1; top++;
            stack[top].x = px; stack[top].y = py - 1; top++;
        }
    }
    
    if (memory && memory->scratch.base && stack != fallback_stack) {
        memory->scratch.used = old_scratch_used;
    }
}

void gfxbuf_palette(int attr, int color)
{
 if (attr >= 0 && attr < GFX_MAX_COLORS &&
 color >= 0 && color < 256) {
 palette[attr] = color;
 }
}

 // Render framebuffer to terminal.
 //
 // Uses ANSI 256-color mode and Unicode half-block characters.
 // Each character cell represents 2 vertical pixels.
 //
 // Output format per cell:
 // ESC[38;5;FGm ESC[48;5;BGm <halfblock>
 //
 // Where FG = upper pixel color, BG = lower pixel color.
 // Half-block U+2580 = upper half filled.
 //
 // On Windows, the UTF-8 sequence for U+2580 is E2 96 80.
void gfxbuf_render(void)
{
 if (gw_sdl2_is_active()) {
#ifndef NO_SDL2
     gw_sdl2_present();
     gw_sdl2_poll_events();
#endif
     return;
 }
 int row, col;
 int term_cols = GFX_WIDTH / 4; // 320/4 = 80 columns
 int term_rows = GFX_HEIGHT / 2; // 200/2 = 100 rows
 unsigned char *vpage = framepages[visual_page];

 // Limit to reasonable terminal size
 if (term_rows > 50) term_rows = 50;

 // Move cursor to top-left
 printf("\033[H");

 for (row = 0; row < term_rows; row++) {
 int py_top = row * 2;
 int py_bot = row * 2 + 1;
 int last_fg = -1, last_bg = -1;

 for (col = 0; col < term_cols; col++) {
 int px = col * (GFX_WIDTH / term_cols);
 int fg_idx, bg_idx, fg_ansi, bg_ansi;

 if (px >= GFX_WIDTH) px = GFX_WIDTH - 1;

 fg_idx = (int)vpage[py_top * GFX_WIDTH + px];
 bg_idx = (py_bot < GFX_HEIGHT) ?
 (int)vpage[py_bot * GFX_WIDTH + px] : 0;

 fg_ansi = palette[fg_idx % GFX_MAX_COLORS];
 bg_ansi = palette[bg_idx % GFX_MAX_COLORS];

 if (fg_ansi != last_fg || bg_ansi != last_bg) {
 printf("\033[38;5;%dm\033[48;5;%dm",
 fg_ansi, bg_ansi);
 last_fg = fg_ansi;
 last_bg = bg_ansi;
 }

 if (fg_idx == bg_idx) {
 // Both same: full block or space
 if (fg_idx == 0)
 putchar(' ');
 else
 // Full block U+2588 = E2 96 88
 printf("\xe2\x96\x88");
 } else {
 // Upper half block U+2580 = E2 96 80
 printf("\xe2\x96\x80");
 }
 }
 // Reset colors and newline
 printf("\033[0m\n");
 }
 fflush(stdout);
}

int gfxbuf_active(void)
{
 return gfx_active;
}

void gfxbuf_set_active(int on)
{
 gfx_active = on;
 if (on) {
 gfxbuf_clear(0);
 }
}

// --- Viewport Clipping ---

void gfxbuf_set_viewport(int x1, int y1, int x2, int y2)
{
 int tmp;
 if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
 if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
 if (x1 < 0) x1 = 0;
 if (y1 < 0) y1 = 0;
 if (x2 >= GFX_WIDTH) x2 = GFX_WIDTH - 1;
 if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;
 vp_x1 = x1; vp_y1 = y1;
 vp_x2 = x2; vp_y2 = y2;
}

void gfxbuf_reset_viewport(void)
{
 vp_x1 = 0; vp_y1 = 0;
 vp_x2 = GFX_WIDTH - 1;
 vp_y2 = GFX_HEIGHT - 1;
}

void gfxbuf_get_viewport(int *x1, int *y1,
 int *x2, int *y2)
{
 if (x1) *x1 = vp_x1;
 if (y1) *y1 = vp_y1;
 if (x2) *x2 = vp_x2;
 if (y2) *y2 = vp_y2;
}

void gfxbuf_fill_viewport(int fill_color,
 int border_color)
{
 int x, y;
 // Fill interior
 for (y = vp_y1; y <= vp_y2; y++)
 for (x = vp_x1; x <= vp_x2; x++)
 gfxbuf_pset(x, y, fill_color);
 // Draw border if different from fill
 if (border_color >= 0 &&
 border_color != fill_color) {
 for (x = vp_x1; x <= vp_x2; x++) {
 gfxbuf_pset(x, vp_y1, border_color);
 gfxbuf_pset(x, vp_y2, border_color);
 }
 for (y = vp_y1; y <= vp_y2; y++) {
 gfxbuf_pset(vp_x1, y, border_color);
 gfxbuf_pset(vp_x2, y, border_color);
 }
 }
}

// --- Multi-Page Support ---

void gfxbuf_set_active_page(int page)
{
 if (page < 0) page = 0;
 if (page >= GFX_MAX_PAGES)
 page = GFX_MAX_PAGES - 1;
 active_page = page;
}

void gfxbuf_set_visual_page(int page)
{
 if (page < 0) page = 0;
 if (page >= GFX_MAX_PAGES)
 page = GFX_MAX_PAGES - 1;
 visual_page = page;
}

void gfxbuf_pcopy(int src, int dst)
{
 if (src < 0 || src >= GFX_MAX_PAGES) return;
 if (dst < 0 || dst >= GFX_MAX_PAGES) return;
 if (src == dst) return;
 memcpy(framepages[dst], framepages[src],
 GFX_WIDTH * GFX_HEIGHT);
}

int gfxbuf_get_active_page(void)
{
 return active_page;
}

int gfxbuf_get_visual_page(void)
{
 return visual_page;
}

void gfxbuf_get_cursor(int *x, int *y) {
    if (x) *x = gfx_cursor_x;
    if (y) *y = gfx_cursor_y;
}

void gfxbuf_set_cursor(int x, int y) {
    gfx_cursor_x = x;
    gfx_cursor_y = y;
}
