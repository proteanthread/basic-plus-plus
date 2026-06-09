/*
 * =====================================================================
 * BASIC++ Interpreter - gfxbuf.c
 * =====================================================================
 *
 * Virtual Graphics Framebuffer implementation.
 *
 * RENDERING STRATEGY:
 * The framebuffer stores color indices (0-15) per pixel in a
 * flat byte array. When gfxbuf_render() is called, it outputs
 * ANSI escape sequences with Unicode half-block characters.
 *
 * Each character cell displays 2 vertical pixels:
 * Upper half = pixel at (x, y)
 * Lower half = pixel at (x, y+1)
 *
 * Half-block char: U+2580 (upper half block)
 * The foreground color = upper pixel, background = lower pixel.
 * If both are the same, a full block or space is used.
 *
 * CGA-COMPATIBLE PALETTE:
 * Colors 0-15 map to standard ANSI terminal colors.
 *
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "gfxbuf.h"

/* Pixel buffer: each byte is a color index 0-15 */
static unsigned char framebuf[GFX_WIDTH * GFX_HEIGHT];

/* Palette: maps color indices to ANSI color codes */
static int palette[GFX_MAX_COLORS];

/* Graphics mode active flag */
static int gfx_active = 0;

/* Default CGA palette -> ANSI 256-color indices */
static const int default_palette[16] = {
 0, /* 0: Black */
 4, /* 1: Blue */
 2, /* 2: Green */
 6, /* 3: Cyan */
 1, /* 4: Red */
 5, /* 5: Magenta */
 3, /* 6: Brown */
 7, /* 7: Light Gray */
 8, /* 8: Dark Gray */
 12, /* 9: Light Blue */
 10, /* 10: Light Green */
 14, /* 11: Light Cyan */
 9, /* 12: Light Red */
 13, /* 13: Light Magenta */
 11, /* 14: Yellow */
 15 /* 15: White */
};

void gfxbuf_init(void)
{
 int i;
 memset(framebuf, 0, sizeof(framebuf));
 for (i = 0; i < GFX_MAX_COLORS; i++) {
 palette[i] = default_palette[i];
 }
 gfx_active = 0;
}

void gfxbuf_clear(int color)
{
 if (color < 0) color = 0;
 if (color >= GFX_MAX_COLORS) color = 0;
 memset(framebuf, (unsigned char)color, sizeof(framebuf));
}

void gfxbuf_pset(int x, int y, int color)
{
 if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT)
 return;
 if (color < 0) color = 0;
 if (color >= GFX_MAX_COLORS) color %= GFX_MAX_COLORS;
 framebuf[y * GFX_WIDTH + x] = (unsigned char)color;
}

int gfxbuf_point(int x, int y)
{
 if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT)
 return 0;
 return (int)framebuf[y * GFX_WIDTH + x];
}

/*
 * Bresenham's line algorithm.
 * Draws a line from (x1,y1) to (x2,y2) in the given color.
 */
void gfxbuf_line(int x1, int y1, int x2, int y2, int color)
{
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

/*
 * Midpoint circle algorithm.
 */
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

/*
 * Flood fill using a stack-based iterative approach.
 * Bounded to prevent stack overflow on large areas.
 */
void gfxbuf_paint(int x, int y, int fill_color,
 int border_color)
{
 /* Simple iterative scanline fill */
 /* Use a static stack to avoid dynamic allocation */
#define PAINT_STACK_SIZE 4096
 struct { int x, y; } stack[PAINT_STACK_SIZE];
 int top = 0;
 int old_color;

 if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT)
 return;

 old_color = gfxbuf_point(x, y);
 if (old_color == fill_color) return;
 if (old_color == border_color) return;

 stack[top].x = x;
 stack[top].y = y;
 top++;

 while (top > 0) {
 int px, py, c;
 top--;
 px = stack[top].x;
 py = stack[top].y;

 if (px < 0 || px >= GFX_WIDTH ||
 py < 0 || py >= GFX_HEIGHT)
 continue;

 c = gfxbuf_point(px, py);
 if (c == fill_color || c == border_color)
 continue;

 gfxbuf_pset(px, py, fill_color);

 if (top + 4 < PAINT_STACK_SIZE) {
 stack[top].x = px + 1; stack[top].y = py; top++;
 stack[top].x = px - 1; stack[top].y = py; top++;
 stack[top].x = px; stack[top].y = py + 1; top++;
 stack[top].x = px; stack[top].y = py - 1; top++;
 }
 }
#undef PAINT_STACK_SIZE
}

void gfxbuf_palette(int attr, int color)
{
 if (attr >= 0 && attr < GFX_MAX_COLORS &&
 color >= 0 && color < 256) {
 palette[attr] = color;
 }
}

/*
 * Render framebuffer to terminal.
 *
 * Uses ANSI 256-color mode and Unicode half-block characters.
 * Each character cell represents 2 vertical pixels.
 *
 * Output format per cell:
 * ESC[38;5;FGm ESC[48;5;BGm <halfblock>
 *
 * Where FG = upper pixel color, BG = lower pixel color.
 * Half-block U+2580 = upper half filled.
 *
 * On Windows, the UTF-8 sequence for U+2580 is E2 96 80.
 */
void gfxbuf_render(void)
{
 int row, col;
 int term_cols = GFX_WIDTH / 4; /* 320/4 = 80 columns */
 int term_rows = GFX_HEIGHT / 2; /* 200/2 = 100 rows */

 /* Limit to reasonable terminal size */
 if (term_rows > 50) term_rows = 50;

 /* Move cursor to top-left */
 printf("\033[H");

 for (row = 0; row < term_rows; row++) {
 int py_top = row * 2;
 int py_bot = row * 2 + 1;
 int last_fg = -1, last_bg = -1;

 for (col = 0; col < term_cols; col++) {
 int px = col * (GFX_WIDTH / term_cols);
 int fg_idx, bg_idx, fg_ansi, bg_ansi;

 if (px >= GFX_WIDTH) px = GFX_WIDTH - 1;

 fg_idx = (int)framebuf[py_top * GFX_WIDTH + px];
 bg_idx = (py_bot < GFX_HEIGHT) ?
 (int)framebuf[py_bot * GFX_WIDTH + px] : 0;

 fg_ansi = palette[fg_idx % GFX_MAX_COLORS];
 bg_ansi = palette[bg_idx % GFX_MAX_COLORS];

 if (fg_ansi != last_fg || bg_ansi != last_bg) {
 printf("\033[38;5;%dm\033[48;5;%dm",
 fg_ansi, bg_ansi);
 last_fg = fg_ansi;
 last_bg = bg_ansi;
 }

 if (fg_idx == bg_idx) {
 /* Both same: full block or space */
 if (fg_idx == 0)
 putchar(' ');
 else
 /* Full block U+2588 = E2 96 88 */
 printf("\xe2\x96\x88");
 } else {
 /* Upper half block U+2580 = E2 96 80 */
 printf("\xe2\x96\x80");
 }
 }
 /* Reset colors and newline */
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
