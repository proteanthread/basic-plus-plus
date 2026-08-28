// FILENAME: gfx_draw_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// NEEDS: libkernel (gfx_primitives_internal.h)
// Implements virtual device and graphics rendering logic for gfx_draw_core.
//
// ---- Includes ----

#include "device/gfx_primitives_internal.h"
#include "device/bgi.h"
#include "runtime/string/memops.h"

int g_last_plot_x = 0;
int g_last_plot_y = 0;

//
// ---- Raster Drawing Primitives ----

void gfx_pset(int x, int y, uint32_t col) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_putpixel(bgi, x, y, (int)col);
    } else if (g_pixels && x >= 0 && x < g_width && y >= 0 && y < g_height) {
        g_pixels[y * g_width + x] = col;
    }
}

void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, (int)color);
        BGI_line(bgi, x1, y1, x2, y2);
    } else if (g_pixels) {
        int dx = (x2 >= x1) ? (x2 - x1) : (x1 - x2);
        int dy = (y2 >= y1) ? (y2 - y1) : (y1 - y2);
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
}

void draw_circle(int cx, int cy, int r, uint32_t color) {
    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, (int)color);
        BGI_circle(bgi, cx, cy, r);
    } else if (g_pixels) {
        int x = r;
        int y = 0;
        int err = 0;

        while (x >= y) {
            int pts[8][2] = {
                {cx + x, cy + y}, {cx + y, cy + x},
                {cx - y, cy + x}, {cx - x, cy + y},
                {cx - x, cy - y}, {cx - y, cy - x},
                {cx + y, cy - x}, {cx + x, cy - y}
            };
            for (int p = 0; p < 8; ++p) {
                int px = pts[p][0];
                int py = pts[p][1];
                if (px >= 0 && px < g_width && py >= 0 && py < g_height) {
                    g_pixels[py * g_width + px] = color;
                }
            }

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
}

//
// ---- Scanline Flood Fill ----

BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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

    BGI_Context *bgi_ctx = BGI_get_global_context();
    if (!bgi_ctx->initialized) {
        BGI_init(bgi_ctx);
    }

    if (bgi_ctx->initialized && bgi_ctx->framebuffer) {
        int fill_idx = (paint_color >= 0 && paint_color < 256)
                       ? paint_color : g_fg_color_idx;
        int bord_idx = (border_color >= 0 && border_color < 256)
                       ? border_color : fill_idx;
        BGI_setfillstyle(bgi_ctx, BGI_SOLID_FILL, fill_idx);
        BGI_setcolor(bgi_ctx, bord_idx);
        BGI_floodfill(bgi_ctx, (int)x_val.as.number, (int)y_val.as.number, bord_idx);
        bgi_sync_screen_mode(graphics_mode);

        VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
        if (con) gfx_con_flush(con);
    }

    return err;
}
