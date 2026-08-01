/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file bgi_raster.c
 * @brief BGI Software Rasterizer — Line, circle, ellipse, flood fill, bar.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements all vector drawing primitives using only the
 *   BGI_putpixel() callback. Algorithms are integer-only (Bresenham) for
 *   maximum speed and portability.
 * - Why it exists: Provides hardware-independent drawing that works on any
 *   BGI framebuffer regardless of backend or display device.
 * - Why it works this way: All drawing routes through BGI_putpixel() which
 *   handles viewport clipping, palette resolution, and indexed VRAM writes.
 *   Flood fill uses an iterative scanline span algorithm with a heap-allocated
 *   work stack to avoid C call-stack overflow on large fills.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add new shape primitives by composing putpixel calls.
 * - What cannot be changed: All flood fill must remain iterative (never recursive).
 * - What to expect: Drawing outside the viewport is silently clipped.
 * - What to do if something breaks: Verify BGI context is initialized, verify
 *   framebuffer is allocated.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: BGI_Context is initialized and has an active mode.
 * - Portability concerns: Pure ISO C17. Uses only calloc/free/memset.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add arc, sector, polyline, etc.
 * - How to write external extensions: Compose BGI_putpixel calls.
 */

#include "bpp_bgi.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ======================================================================
 * Internal pixel write helper (raw, no viewport offset — for rasterizer use)
 * ====================================================================== */
static void raster_putpixel(BGI_Context *ctx, int x, int y, int color) {
    BGI_putpixel(ctx, x, y, color);
}

/* ======================================================================
 * BGI_line — Bresenham integer line algorithm
 * ====================================================================== */
void BGI_line(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    int color = ctx->fg_color;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        raster_putpixel(ctx, x1, y1, color);
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

    /* Update current position */
    ctx->cp_x = x2;
    ctx->cp_y = y2;
}

/* ======================================================================
 * BGI_lineto — Draw from current position to (x,y)
 * ====================================================================== */
void BGI_lineto(BGI_Context *ctx, int x, int y) {
    if (!ctx) return;
    BGI_line(ctx, ctx->cp_x, ctx->cp_y, x, y);
}

/* ======================================================================
 * BGI_circle — Bresenham midpoint circle algorithm
 * ====================================================================== */
void BGI_circle(BGI_Context *ctx, int cx, int cy, int r) {
    if (!ctx || !ctx->framebuffer || r < 0) return;

    int color = ctx->fg_color;
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        raster_putpixel(ctx, cx + x, cy + y, color);
        raster_putpixel(ctx, cx + y, cy + x, color);
        raster_putpixel(ctx, cx - y, cy + x, color);
        raster_putpixel(ctx, cx - x, cy + y, color);
        raster_putpixel(ctx, cx - x, cy - y, color);
        raster_putpixel(ctx, cx - y, cy - x, color);
        raster_putpixel(ctx, cx + y, cy - x, color);
        raster_putpixel(ctx, cx + x, cy - y, color);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

/* ======================================================================
 * BGI_ellipse — Midpoint ellipse with arc clipping (angle in degrees)
 * ====================================================================== */
void BGI_ellipse(BGI_Context *ctx, int cx, int cy, int sa, int ea,
                 int rx, int ry) {
    if (!ctx || !ctx->framebuffer) return;
    if (rx < 0 || ry < 0) return;

    int color = ctx->fg_color;
    double pi = 3.14159265358979323846;
    double start_rad = (double)sa * pi / 180.0;
    double end_rad = (double)ea * pi / 180.0;

    /* Normalize angles */
    if (end_rad < start_rad) end_rad += 2.0 * pi;

    /* Parametric plot with angular step */
    int steps = (rx + ry) * 2;
    if (steps < 64) steps = 64;

    for (int i = 0; i <= steps; ++i) {
        double t = start_rad + (end_rad - start_rad) * (double)i / (double)steps;
        int px = cx + (int)(cos(t) * (double)rx + 0.5);
        int py = cy - (int)(sin(t) * (double)ry + 0.5);
        raster_putpixel(ctx, px, py, color);
    }
}

/* ======================================================================
 * BGI_fillellipse — Filled ellipse using scanline horizontal spans
 * ====================================================================== */
void BGI_fillellipse(BGI_Context *ctx, int cx, int cy, int rx, int ry) {
    if (!ctx || !ctx->framebuffer) return;
    if (rx < 0 || ry < 0) return;

    int color = ctx->fill_color;

    for (int y = -ry; y <= ry; ++y) {
        /* Calculate x extent at this y using ellipse equation */
        double x_extent = (double)rx * sqrt(1.0 - ((double)(y * y)) /
                          ((double)(ry * ry) + 0.0001));
        int x_start = (int)(-x_extent);
        int x_end = (int)(x_extent);
        for (int x = x_start; x <= x_end; ++x) {
            raster_putpixel(ctx, cx + x, cy + y, color);
        }
    }
}

/* ======================================================================
 * BGI_bar — Filled rectangle
 * ====================================================================== */
void BGI_bar(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    int color = ctx->fill_color;

    /* Normalize coordinates */
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            raster_putpixel(ctx, x, y, color);
        }
    }
}

/* ======================================================================
 * BGI_rectangle — Unfilled rectangle outline
 * ====================================================================== */
void BGI_rectangle(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    /* Save and use fg_color for outline */
    int saved_cp_x = ctx->cp_x;
    int saved_cp_y = ctx->cp_y;

    BGI_line(ctx, x1, y1, x2, y1); /* Top    */
    BGI_line(ctx, x2, y1, x2, y2); /* Right  */
    BGI_line(ctx, x2, y2, x1, y2); /* Bottom */
    BGI_line(ctx, x1, y2, x1, y1); /* Left   */

    ctx->cp_x = saved_cp_x;
    ctx->cp_y = saved_cp_y;
}

/* ======================================================================
 * BGI_floodfill — Iterative scanline span-fill (NEVER recursive)
 *
 * Uses a heap-allocated work stack of horizontal spans.
 * ====================================================================== */
typedef struct {
    int32_t x1;
    int32_t x2;
    int32_t y;
    int32_t dir; /* +1 = scan downward, -1 = scan upward */
} BGI_FillSpan;

void BGI_floodfill(BGI_Context *ctx, int seed_x, int seed_y, int border) {
    if (!ctx || !ctx->framebuffer) return;

    int fill_color = ctx->fill_color;
    uint32_t border_argb = BGI_resolve_color(ctx, border);
    uint32_t fill_argb = BGI_resolve_color(ctx, fill_color);

    /* Check seed point */
    int sx = seed_x + ctx->vp_left;
    int sy = seed_y + ctx->vp_top;
    if (sx < 0 || sx >= ctx->fb_width || sy < 0 || sy >= ctx->fb_height) return;

    uint32_t seed_color = ctx->framebuffer[sy * ctx->fb_width + sx];
    if (seed_color == border_argb || seed_color == fill_argb) return;

    /* Allocate span stack */
    size_t capacity = 256;
    size_t count = 0;
    BGI_FillSpan *stack = (BGI_FillSpan *)calloc(capacity, sizeof(BGI_FillSpan));
    if (!stack) return;

    /* Push initial span */
    stack[count].x1 = seed_x;
    stack[count].x2 = seed_x;
    stack[count].y = seed_y;
    stack[count].dir = 1;
    count++;
    stack[count].x1 = seed_x;
    stack[count].x2 = seed_x;
    stack[count].y = seed_y - 1;
    stack[count].dir = -1;
    count++;

    int max_x = ctx->vp_right - ctx->vp_left;
    int max_y = ctx->vp_bottom - ctx->vp_top;

    while (count > 0) {
        /* Pop */
        count--;
        BGI_FillSpan span = stack[count];
        int y = span.y;
        int x1 = span.x1;
        int x2 = span.x2;

        if (y < 0 || y > max_y) continue;

        /* Find the left extent */
        int lx = x1;
        {
            int px = lx + ctx->vp_left;
            int py = y + ctx->vp_top;
            while (lx > 0) {
                px = (lx - 1) + ctx->vp_left;
                if (px < 0 || px >= ctx->fb_width) break;
                uint32_t c = ctx->framebuffer[py * ctx->fb_width + px];
                if (c == border_argb || c == fill_argb) break;
                lx--;
            }
        }

        /* Find the right extent */
        int rx = x2;
        {
            int py = y + ctx->vp_top;
            while (rx < max_x) {
                int px = (rx + 1) + ctx->vp_left;
                if (px < 0 || px >= ctx->fb_width) break;
                uint32_t c = ctx->framebuffer[py * ctx->fb_width + px];
                if (c == border_argb || c == fill_argb) break;
                rx++;
            }
        }

        /* Fill the span */
        for (int x = lx; x <= rx; ++x) {
            raster_putpixel(ctx, x, y, fill_color);
        }

        /* Scan above and below for new spans to push */
        int dirs[2] = { 1, -1 };
        for (int d = 0; d < 2; ++d) {
            int ny = y + dirs[d];
            if (ny < 0 || ny > max_y) continue;

            int py = ny + ctx->vp_top;
            if (py < 0 || py >= ctx->fb_height) continue;

            int scan_x = lx;
            while (scan_x <= rx) {
                /* Find start of unfilled segment */
                int px = scan_x + ctx->vp_left;
                if (px < 0 || px >= ctx->fb_width) { scan_x++; continue; }
                uint32_t c = ctx->framebuffer[py * ctx->fb_width + px];
                if (c == border_argb || c == fill_argb) {
                    scan_x++;
                    continue;
                }

                /* Found start of new span */
                int span_start = scan_x;
                while (scan_x <= rx) {
                    px = scan_x + ctx->vp_left;
                    if (px < 0 || px >= ctx->fb_width) break;
                    c = ctx->framebuffer[py * ctx->fb_width + px];
                    if (c == border_argb || c == fill_argb) break;
                    scan_x++;
                }

                /* Push this span */
                if (count >= capacity) {
                    size_t new_cap = capacity * 2;
                    BGI_FillSpan *new_stack = (BGI_FillSpan *)calloc(
                        new_cap, sizeof(BGI_FillSpan));
                    if (!new_stack) { free(stack); return; }
                    memcpy(new_stack, stack, count * sizeof(BGI_FillSpan));
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
}

/* ======================================================================
 * BGI_getimage / BGI_putimage / BGI_imagesize
 * ====================================================================== */

size_t BGI_imagesize(int x1, int y1, int x2, int y2) {
    if (x2 < x1 || y2 < y1) return 0;
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    /* Header: 2 ints (width, height) + pixel data */
    return sizeof(int) * 2 + (size_t)w * (size_t)h * sizeof(uint32_t);
}

void BGI_getimage(const BGI_Context *ctx, int x1, int y1, int x2, int y2,
                  void *buffer) {
    if (!ctx || !ctx->framebuffer || !buffer) return;
    if (x2 < x1 || y2 < y1) return;

    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;

    int *header = (int *)buffer;
    header[0] = w;
    header[1] = h;

    uint32_t *pixels = (uint32_t *)((char *)buffer + sizeof(int) * 2);
    int idx = 0;
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            int px = x + ctx->vp_left;
            int py = y + ctx->vp_top;
            if (px >= 0 && px < ctx->fb_width && py >= 0 && py < ctx->fb_height) {
                pixels[idx] = ctx->framebuffer[py * ctx->fb_width + px];
            } else {
                pixels[idx] = 0xFF000000u;
            }
            idx++;
        }
    }
}

void BGI_putimage(BGI_Context *ctx, int x, int y, const void *buffer, int op) {
    if (!ctx || !ctx->framebuffer || !buffer) return;

    const int *header = (const int *)buffer;
    int w = header[0];
    int h = header[1];
    const uint32_t *pixels = (const uint32_t *)((const char *)buffer +
                              sizeof(int) * 2);

    int idx = 0;
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            int px = (x + dx) + ctx->vp_left;
            int py = (y + dy) + ctx->vp_top;
            if (px >= 0 && px < ctx->fb_width &&
                py >= 0 && py < ctx->fb_height) {
                uint32_t src = pixels[idx];
                uint32_t *dst = &ctx->framebuffer[py * ctx->fb_width + px];
                switch (op) {
                    case BGI_COPY_PUT: *dst = src; break;
                    case BGI_XOR_PUT:  *dst ^= src; break;
                    case BGI_OR_PUT:   *dst |= src; break;
                    case BGI_AND_PUT:  *dst &= src; break;
                    case BGI_NOT_PUT:  *dst = ~src; break;
                    default:           *dst = src; break;
                }
            }
            idx++;
        }
    }
}
