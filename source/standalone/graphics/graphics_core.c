/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: graphics_core.c
 * Subsystem: Portable Software Rasterizer Engine
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Performs software rasterization (lines, circles, flood fills).
 *
 * 2. WHAT TO EXPECT:
 *    Renders geometric shapes directly into a screen buffer.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Resolution boundaries, clipping coordinates.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Bresenham line and circle algorithms.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If shapes draw offscreen, verify viewport bounds.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE SOFTWARE RASTERIZER ENGINE
 * File: graphics_core.c
 * ===================================================================== */

#include "graphics_core.h"
#include <string.h>
#include <stdio.h>

void graphics_core_clear(GraphicsCanvas *canvas, int color)
{
    if (canvas && canvas->pixels) {
        memset(canvas->pixels, (unsigned char)color, canvas->width * canvas->height);
    }
}

void graphics_core_pset(GraphicsCanvas *canvas, int x, int y, int color)
{
    if (!canvas || !canvas->pixels) return;
    if (x < canvas->clip_x1 || x > canvas->clip_x2 ||
        y < canvas->clip_y1 || y > canvas->clip_y2) return;
    if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) return;
    canvas->pixels[y * canvas->width + x] = (unsigned char)color;
}

int graphics_core_point(const GraphicsCanvas *canvas, int x, int y)
{
    if (!canvas || !canvas->pixels) return 0;
    if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) return 0;
    return (int)canvas->pixels[y * canvas->width + x];
}

void graphics_core_line(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color)
{
    int dx, dy, sx, sy, err, e2;

    dx = x2 - x1; if (dx < 0) dx = -dx;
    dy = y2 - y1; if (dy < 0) dy = -dy;
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    for (;;) {
        graphics_core_pset(canvas, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void graphics_core_box(GraphicsCanvas *canvas, int x1, int y1, int x2, int y2, int color, int filled)
{
    int x, y, tmp;

    if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
    if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }

    if (filled) {
        for (y = y1; y <= y2; y++) {
            for (x = x1; x <= x2; x++) {
                graphics_core_pset(canvas, x, y, color);
            }
        }
    } else {
        for (x = x1; x <= x2; x++) {
            graphics_core_pset(canvas, x, y1, color);
            graphics_core_pset(canvas, x, y2, color);
        }
        for (y = y1; y <= y2; y++) {
            graphics_core_pset(canvas, x1, y, color);
            graphics_core_pset(canvas, x2, y, color);
        }
    }
}

void graphics_core_circle(GraphicsCanvas *canvas, int cx, int cy, int r, int color)
{
    int x = 0, y = r;
    int d = 1 - r;

    while (x <= y) {
        graphics_core_pset(canvas, cx + x, cy + y, color);
        graphics_core_pset(canvas, cx - x, cy + y, color);
        graphics_core_pset(canvas, cx + x, cy - y, color);
        graphics_core_pset(canvas, cx - x, cy - y, color);
        graphics_core_pset(canvas, cx + y, cy + x, color);
        graphics_core_pset(canvas, cx - y, cy + x, color);
        graphics_core_pset(canvas, cx + y, cy - x, color);
        graphics_core_pset(canvas, cx - y, cy - x, color);
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void graphics_core_paint(GraphicsCanvas *canvas, int x, int y, int fill_color, int border_color,
                          GraphicsCorePoint *stack, int stack_capacity)
{
    int top = 0;
    int old_color;

    if (!canvas || !canvas->pixels || !stack) return;
    if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) return;

    old_color = graphics_core_point(canvas, x, y);
    if (old_color == fill_color || old_color == border_color) {
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

        if (px < 0 || px >= canvas->width || py < 0 || py >= canvas->height)
            continue;

        c = graphics_core_point(canvas, px, py);
        if (c == fill_color || c == border_color)
            continue;

        graphics_core_pset(canvas, px, py, fill_color);

        if (top + 4 < stack_capacity) {
            stack[top].x = px + 1; stack[top].y = py; top++;
            stack[top].x = px - 1; stack[top].y = py; top++;
            stack[top].x = px; stack[top].y = py + 1; top++;
            stack[top].x = px; stack[top].y = py - 1; top++;
        }
    }
}

void graphics_core_render_ansi(const GraphicsCanvas *canvas,
                               const int *palette, int max_colors,
                               GraphicsCorePrintStrCallback print_str,
                               GraphicsCorePrintCharCallback print_char,
                               void *user_data)
{
    int row, col;
    int term_cols = canvas->width / 4;
    int term_rows = canvas->height / 2;
    char esc_buf[64];

    if (!canvas || !canvas->pixels || !print_str || !print_char) return;

    if (term_rows > 50) term_rows = 50;

    print_str(user_data, "\033[H");

    for (row = 0; row < term_rows; row++) {
        int py_top = row * 2;
        int py_bot = row * 2 + 1;
        int last_fg = -1, last_bg = -1;

        for (col = 0; col < term_cols; col++) {
            int px = col * (canvas->width / term_cols);
            int fg_idx, bg_idx, fg_ansi, bg_ansi;

            if (px >= canvas->width) px = canvas->width - 1;

            fg_idx = (int)canvas->pixels[py_top * canvas->width + px];
            bg_idx = (py_bot < canvas->height) ? (int)canvas->pixels[py_bot * canvas->width + px] : 0;

            fg_ansi = palette[fg_idx % max_colors];
            bg_ansi = palette[bg_idx % max_colors];

            if (fg_ansi != last_fg || bg_ansi != last_bg) {
                /* Format ANSI code */
                #ifdef _MSC_VER
                sprintf_s(esc_buf, sizeof(esc_buf), "\033[38;5;%dm\033[48;5;%dm", fg_ansi, bg_ansi);
                #else
                sprintf(esc_buf, "\033[38;5;%dm\033[48;5;%dm", fg_ansi, bg_ansi);
                #endif
                print_str(user_data, esc_buf);
                last_fg = fg_ansi;
                last_bg = bg_ansi;
            }

            if (fg_idx == bg_idx) {
                if (fg_idx == 0) {
                    print_char(user_data, ' ');
                } else {
                    print_str(user_data, "\xe2\x96\x88");
                }
            } else {
                print_str(user_data, "\xe2\x96\x80");
            }
        }
        print_str(user_data, "\033[0m\n");
    }
}
