// FILENAME: bgi_raster.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (hal.h, math.h, memops.h, memops.c)
// NEEDS: libengine (bgi.h, bgi.c, math.c)
// Implements AST parsing and evaluation structures for bgi_raster.
//
// ---- Includes ----

#include "device/bgi.h"
#include "hal/hal.h"
#include "runtime/math/math.h"
#include "runtime/string/memops.h"

// ======================================================================
// Internal pixel write helper (raw, no viewport offset — for rasterizer use)
// ======================================================================
static void raster_putpixel(BGI_Context *ctx, int x, int y, int color) {
    BGI_putpixel(ctx, x, y, color);
}

// ======================================================================
// BGI_line — Bresenham integer line algorithm
// ======================================================================
void BGI_line(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    int color = ctx->fg_color;
    int dx = (int)runtime_abs(x2 - x1);
    int dy = (int)runtime_abs(y2 - y1);
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

    // Update current position
    ctx->cp_x = x2;
    ctx->cp_y = y2;
}

// ======================================================================
// BGI_lineto — Draw from current position to (x,y)
// ======================================================================
void BGI_lineto(BGI_Context *ctx, int x, int y) {
    if (!ctx) return;
    BGI_line(ctx, ctx->cp_x, ctx->cp_y, x, y);
}

// ======================================================================
// BGI_circle — Bresenham midpoint circle algorithm
// ======================================================================
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

// ======================================================================
// BGI_ellipse — Midpoint ellipse with arc clipping (angle in degrees)
// ======================================================================
void BGI_ellipse(BGI_Context *ctx, int cx, int cy, int sa, int ea,
                 int rx, int ry) {
    if (!ctx || !ctx->framebuffer) return;
    if (rx < 0 || ry < 0) return;

    int color = ctx->fg_color;
    double pi = 3.14159265358979323846;
    double start_rad = (double)sa * pi / 180.0;
    double end_rad = (double)ea * pi / 180.0;

    // Normalize angles
    if (end_rad < start_rad) end_rad += 2.0 * pi;

    // Parametric plot with angular step
    int steps = (rx + ry) * 2;
    if (steps < 64) steps = 64;

    for (int i = 0; i <= steps; ++i) {
        double t = start_rad + (end_rad - start_rad) * (double)i / (double)steps;
        int px = cx + (int)(runtime_cos(t) * (double)rx + 0.5);
        int py = cy - (int)(runtime_sin(t) * (double)ry + 0.5);
        raster_putpixel(ctx, px, py, color);
    }
}

// ======================================================================
// BGI_fillellipse — Filled ellipse using scanline horizontal spans
// ======================================================================
void BGI_fillellipse(BGI_Context *ctx, int cx, int cy, int rx, int ry) {
    if (!ctx || !ctx->framebuffer) return;
    if (rx < 0 || ry < 0) return;

    int color = ctx->fill_color;

    for (int y = -ry; y <= ry; ++y) {
        // Calculate x extent at this y using ellipse equation
        double x_extent = (double)rx * runtime_sqrt(1.0 - ((double)(y * y)) /
                          ((double)(ry * ry) + 0.0001));
        int x_start = (int)(-x_extent);
        int x_end = (int)(x_extent);
        for (int x = x_start; x <= x_end; ++x) {
            raster_putpixel(ctx, cx + x, cy + y, color);
        }
    }
}


// ======================================================================
// BGI_bar — Filled rectangle
// ======================================================================
void BGI_bar(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    int color = ctx->fill_color;

    // Normalize coordinates
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            raster_putpixel(ctx, x, y, color);
        }
    }
}

// ======================================================================
// BGI_rectangle — Unfilled rectangle outline
// ======================================================================
void BGI_rectangle(BGI_Context *ctx, int x1, int y1, int x2, int y2) {
    if (!ctx || !ctx->framebuffer) return;

    // Save and use fg_color for outline
    int saved_cp_x = ctx->cp_x;
    int saved_cp_y = ctx->cp_y;

    BGI_line(ctx, x1, y1, x2, y1); // Top
    BGI_line(ctx, x2, y1, x2, y2); // Right
    BGI_line(ctx, x2, y2, x1, y2); // Bottom
    BGI_line(ctx, x1, y2, x1, y1); // Left

    ctx->cp_x = saved_cp_x;
    ctx->cp_y = saved_cp_y;
}

// ======================================================================
// BGI_floodfill — Iterative scanline span-fill (NEVER recursive)
//
// Uses a heap-allocated work stack of horizontal spans.
// ======================================================================
typedef struct {
    int32_t x1;
    int32_t x2;
    int32_t y;
    int32_t dir; // +1 = scan downward, -1 = scan upward
} BGI_FillSpan;

void BGI_floodfill(BGI_Context *ctx, int seed_x, int seed_y, int border) {
    if (!ctx || !ctx->framebuffer) return;

    int fill_color = ctx->fill_color;
    uint32_t border_argb = BGI_resolve_color(ctx, border);
    uint32_t fill_argb = BGI_resolve_color(ctx, fill_color);

    // Check seed point
    int sx = seed_x + ctx->vp_left;
    int sy = seed_y + ctx->vp_top;
    if (sx < 0 || sx >= ctx->fb_width || sy < 0 || sy >= ctx->fb_height) return;

    uint32_t seed_color = ctx->framebuffer[sy * ctx->fb_width + sx];
    if (seed_color == border_argb || seed_color == fill_argb) return;

    // Allocate span stack
    HalContext *hal = hal_get();
    size_t capacity = 256;
    size_t count = 0;
    size_t span_bytes = capacity * sizeof(BGI_FillSpan);
    BGI_FillSpan *stack = (BGI_FillSpan *)(hal && hal->mem.alloc ? hal->mem.alloc(span_bytes) : NULL);
    if (!stack) return;
    runtime_memset(stack, 0, span_bytes);

    // Push initial span
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
        // Pop
        count--;
        BGI_FillSpan span = stack[count];
        int y = span.y;
        int x1 = span.x1;
        int x2 = span.x2;

        if (y < 0 || y > max_y) continue;

        // Find the left extent
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

        // Find the right extent
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

        // Fill the span
        for (int x = lx; x <= rx; ++x) {
            raster_putpixel(ctx, x, y, fill_color);
        }

        // Scan above and below for new spans to push
        int dirs[2] = { 1, -1 };
        for (int d = 0; d < 2; ++d) {
            int ny = y + dirs[d];
            if (ny < 0 || ny > max_y) continue;

            int py = ny + ctx->vp_top;
            if (py < 0 || py >= ctx->fb_height) continue;

            int scan_x = lx;
            while (scan_x <= rx) {
                // Find start of unfilled segment
                int px = scan_x + ctx->vp_left;
                if (px < 0 || px >= ctx->fb_width) { scan_x++; continue; }
                uint32_t c = ctx->framebuffer[py * ctx->fb_width + px];
                if (c == border_argb || c == fill_argb) {
                    scan_x++;
                    continue;
                }

                // Found start of new span
                int span_start = scan_x;
                while (scan_x <= rx) {
                    px = scan_x + ctx->vp_left;
                    if (px < 0 || px >= ctx->fb_width) break;
                    c = ctx->framebuffer[py * ctx->fb_width + px];
                    if (c == border_argb || c == fill_argb) break;
                    scan_x++;
                }

                // Push this span
                if (count >= capacity) {
                    size_t new_cap = capacity * 2;
                    size_t new_bytes = new_cap * sizeof(BGI_FillSpan);
                    BGI_FillSpan *new_stack = (BGI_FillSpan *)(hal && hal->mem.alloc ? hal->mem.alloc(new_bytes) : NULL);
                    if (!new_stack) {
                        if (hal && hal->mem.free) hal->mem.free(stack);
                        return;
                    }
                    runtime_memset(new_stack, 0, new_bytes);
                    runtime_memcpy(new_stack, stack, count * sizeof(BGI_FillSpan));
                    if (hal && hal->mem.free) hal->mem.free(stack);
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

    if (hal && hal->mem.free) hal->mem.free(stack);
}


// ======================================================================
// BGI_getimage / BGI_putimage / BGI_imagesize
// ======================================================================

size_t BGI_imagesize(int x1, int y1, int x2, int y2) {
    if (x2 < x1 || y2 < y1) return 0;
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    // Header: 2 ints (width, height) + pixel data
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

// ======================================================================
// SECTION Q: DRAW Macro Vector Language Interpreter
// ======================================================================

void BGI_draw(BGI_Context *ctx, const char *command_str) {
    if (!ctx || !ctx->framebuffer || !command_str) return;

    int cur_x = ctx->cp_x;
    int cur_y = ctx->cp_y;
    int scale = 4; // Default scale: 4 = 1:1 scale (unit length = 1)
    int angle = 0; // 0=0 deg, 1=90, 2=180, 3=270

    const char *p = command_str;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t' || *p == ';') p++;
        if (*p == '\0') break;

        bool move_only = false;
        bool return_pos = false;

        // Check for prefixes: B (blind move), N (no update)
        while (*p == 'B' || *p == 'b' || *p == 'N' || *p == 'n') {
            if (*p == 'B' || *p == 'b') move_only = true;
            if (*p == 'N' || *p == 'n') return_pos = true;
            p++;
        }

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        char cmd = (char)((*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p);
        p++;

        int start_x = cur_x;
        int start_y = cur_y;
        int target_x = cur_x;
        int target_y = cur_y;

        if (cmd == 'U' || cmd == 'D' || cmd == 'L' || cmd == 'R' ||
            cmd == 'E' || cmd == 'F' || cmd == 'G' || cmd == 'H') {
            int n = 1;
            while (*p == ' ' || *p == '\t') p++;
            if (*p >= '0' && *p <= '9') {
                n = 0;
                while (*p >= '0' && *p <= '9') {
                    n = n * 10 + (*p - '0');
                    p++;
                }
            }
            int dist = (n * scale) / 4;
            if (dist < 1) dist = 1;

            int dx = 0, dy = 0;
            if (cmd == 'U') { dy = -dist; }
            else if (cmd == 'D') { dy = dist; }
            else if (cmd == 'L') { dx = -dist; }
            else if (cmd == 'R') { dx = dist; }
            else if (cmd == 'E') { dx = dist; dy = -dist; }
            else if (cmd == 'F') { dx = dist; dy = dist; }
            else if (cmd == 'G') { dx = -dist; dy = dist; }
            else if (cmd == 'H') { dx = -dist; dy = -dist; }

            // Apply rotation angle (0, 90, 180, 270)
            if (angle == 1) { int t = dx; dx = -dy; dy = t; }
            else if (angle == 2) { dx = -dx; dy = -dy; }
            else if (angle == 3) { int t = dx; dx = dy; dy = -t; }

            target_x = cur_x + dx;
            target_y = cur_y + dy;

            if (!move_only) {
                BGI_line(ctx, cur_x, cur_y, target_x, target_y);
            }
            if (return_pos) {
                cur_x = start_x;
                cur_y = start_y;
            } else {
                cur_x = target_x;
                cur_y = target_y;
            }
        } else if (cmd == 'M') {
            while (*p == ' ' || *p == '\t') p++;
            bool rel_x = false;
            int sign_x = 1;
            if (*p == '+') { rel_x = true; p++; }
            else if (*p == '-') { rel_x = true; sign_x = -1; p++; }

            int mx = 0;
            while (*p >= '0' && *p <= '9') {
                mx = mx * 10 + (*p - '0');
                p++;
            }
            mx *= sign_x;

            while (*p == ' ' || *p == '\t') p++;
            if (*p == ',') p++;
            while (*p == ' ' || *p == '\t') p++;

            bool rel_y = false;
            int sign_y = 1;
            if (*p == '+') { rel_y = true; p++; }
            else if (*p == '-') { rel_y = true; sign_y = -1; p++; }

            int my = 0;
            while (*p >= '0' && *p <= '9') {
                my = my * 10 + (*p - '0');
                p++;
            }
            my *= sign_y;

            target_x = rel_x ? (cur_x + mx) : mx;
            target_y = rel_y ? (cur_y + my) : my;

            if (!move_only) {
                BGI_line(ctx, cur_x, cur_y, target_x, target_y);
            }
            if (return_pos) {
                cur_x = start_x;
                cur_y = start_y;
            } else {
                cur_x = target_x;
                cur_y = target_y;
            }
        } else if (cmd == 'C') {
            while (*p == ' ' || *p == '\t') p++;
            int col = 0;
            while (*p >= '0' && *p <= '9') {
                col = col * 10 + (*p - '0');
                p++;
            }
            BGI_setcolor(ctx, col);
        } else if (cmd == 'S') {
            while (*p == ' ' || *p == '\t') p++;
            int s = 4;
            if (*p >= '0' && *p <= '9') {
                s = 0;
                while (*p >= '0' && *p <= '9') {
                    s = s * 10 + (*p - '0');
                    p++;
                }
            }
            if (s > 0) scale = s;
        } else if (cmd == 'A') {
            while (*p == ' ' || *p == '\t') p++;
            if (*p >= '0' && *p <= '3') {
                angle = *p - '0';
                p++;
            }
        } else {
            // Unrecognized command or parameter, advance
            p++;
        }
    }

    ctx->cp_x = cur_x;
    ctx->cp_y = cur_y;
}

