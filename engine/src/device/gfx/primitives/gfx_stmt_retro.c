// FILENAME: gfx_stmt_retro.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// NEEDS: libkernel (gfx_primitives_internal.h)
// Implements virtual device and graphics rendering logic for gfx_stmt_retro.
//
// ---- Includes ----

#include "device/gfx_primitives_internal.h"
#include "device/bgi.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

//
// ---- Apple II Graphics Statements ----

BppError vdev_legacy_stmt_gr_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 40, 48);
    if (err.code == 0) {
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized && bgi->framebuffer) {
            BGI_cleardevice(bgi);
            BGI_present(bgi);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_hgr_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 280, 192);
    if (err.code == 0) {
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized && bgi->framebuffer) {
            BGI_cleardevice(bgi);
            BGI_present(bgi);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_hgr2_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = init_graphics_mode_dims(vm, 280, 192);
    if (err.code == 0) {
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized && bgi->framebuffer) {
            BGI_cleardevice(bgi);
            BGI_present(bgi);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_hcolor_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EQ) {
        lex_next(lex);
    }
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "HCOLOR expects a numeric value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        g_fg_color_idx = color;
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized) {
            BGI_setcolor(bgi, color);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_plot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_putpixel(bgi, x, y, g_fg_color_idx);
        BGI_present(bgi);
    } else {
        gfx_pset(x, y, (uint32_t)g_fg_color_idx);
    }
    return err;
}

BppError vdev_legacy_stmt_hlin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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
    if (tok.type != TOK_IDENT || tok.length != 2 || runtime_strncasecmp(tok.start, "AT", 2) != 0) {
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

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, g_fg_color_idx);
        BGI_line(bgi, x1, y, x2, y);
        BGI_present(bgi);
    } else {
        int start = (x1 < x2) ? x1 : x2;
        int end = (x1 < x2) ? x2 : x1;
        for (int x = start; x <= end; x++) {
            gfx_pset(x, y, (uint32_t)g_fg_color_idx);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_vlin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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
    if (tok.type != TOK_IDENT || tok.length != 2 || runtime_strncasecmp(tok.start, "AT", 2) != 0) {
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

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, g_fg_color_idx);
        BGI_line(bgi, x, y1, x, y2);
        BGI_present(bgi);
    } else {
        int start = (y1 < y2) ? y1 : y2;
        int end = (y1 < y2) ? y2 : y1;
        for (int y = start; y <= end; y++) {
            gfx_pset(x, y, (uint32_t)g_fg_color_idx);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_hplot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) {
        lex_next(lex);
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

        if (bgi->initialized && bgi->framebuffer) {
            BGI_setcolor(bgi, g_fg_color_idx);
            BGI_line(bgi, g_last_plot_x, g_last_plot_y, x, y);
            BGI_present(bgi);
        } else {
            draw_line(g_last_plot_x, g_last_plot_y, x, y, (uint32_t)g_fg_color_idx);
        }
        g_last_plot_x = x;
        g_last_plot_y = y;
    } else {
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

        if (bgi->initialized && bgi->framebuffer) {
            BGI_putpixel(bgi, x, y, g_fg_color_idx);
            BGI_present(bgi);
        } else {
            gfx_pset(x, y, (uint32_t)g_fg_color_idx);
        }
        g_last_plot_x = x;
        g_last_plot_y = y;

        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) {
                lex_next(lex);
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

                if (bgi->initialized && bgi->framebuffer) {
                    BGI_setcolor(bgi, g_fg_color_idx);
                    BGI_line(bgi, g_last_plot_x, g_last_plot_y, nx, ny);
                    BGI_present(bgi);
                } else {
                    draw_line(g_last_plot_x, g_last_plot_y, nx, ny, (uint32_t)g_fg_color_idx);
                }
                g_last_plot_x = nx;
                g_last_plot_y = ny;
            } else {
                break;
            }
        }
    }
    return err;
}

//
// ---- Atari 8-Bit and Sinclair Statements ----

BppError vdev_legacy_stmt_graphics_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (mode_val.type == VAL_STRING) {
        err.code = 13; err.message = "GRAPHICS mode must be numeric";
        return err;
    }
    int mode = (int)mode_val.as.number;
    int base_mode = mode & 0x0F;
    bool fullscreen = (mode & 0x10) != 0;
    (void)fullscreen;

    int w = 320, h = 192;
    if (base_mode == 0) { w = 320; h = 240; }
    else if (base_mode == 3) { w = 40; h = 24; }
    else if (base_mode == 5) { w = 80; h = 48; }
    else if (base_mode == 7) { w = 160; h = 96; }
    else if (base_mode == 8) { w = 320; h = 192; }

    err = init_graphics_mode_dims(vm, w, h);
    if (err.code == 0) {
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized && bgi->framebuffer) {
            BGI_cleardevice(bgi);
            BGI_present(bgi);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_drawto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, g_fg_color_idx);
        BGI_line(bgi, g_last_plot_x, g_last_plot_y, x, y);
        BGI_present(bgi);
    } else {
        draw_line(g_last_plot_x, g_last_plot_y, x, y, (uint32_t)g_fg_color_idx);
    }
    g_last_plot_x = x;
    g_last_plot_y = y;
    return err;
}

BppError vdev_legacy_stmt_border_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "BORDER expects a numeric color value";
        return err;
    }
    (void)val;
    return err;
}

BppError vdev_legacy_stmt_ink_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "INK expects a numeric color value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        g_fg_color_idx = color;
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized) {
            BGI_setcolor(bgi, color);
        }
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_fg_map[16] = {30,34,32,36,31,35,33,37,90,94,92,96,91,95,93,97};
            int fg_ansi = (color < 16) ? ansi_fg_map[color] : color;
            char esc[64];
            if (color < 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[%dm", fg_ansi);
            } else {
                runtime_snprintf(esc, sizeof(esc), "\033[38;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}

BppError vdev_legacy_stmt_paper_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        err.code = 13; err.message = "PAPER expects a numeric color value";
        return err;
    }
    int color = (int)val.as.number;
    if (color >= 0 && color < 256) {
        g_bg_color_idx = color;
        BGI_Context *bgi = BGI_get_global_context();
        if (bgi && bgi->initialized) {
            BGI_setbkcolor(bgi, color);
        }
        VDevContext *vdev_ctx = vm_get_vdev(vm);
        if (vdev_ctx) {
            static const int ansi_bg_map[16] = {40,44,42,46,41,45,43,47,100,104,102,106,101,105,103,107};
            int bg_ansi = (color < 16) ? ansi_bg_map[color] : color;
            char esc[64];
            if (color < 16) {
                runtime_snprintf(esc, sizeof(esc), "\033[%dm", bg_ansi);
            } else {
                runtime_snprintf(esc, sizeof(esc), "\033[48;5;%dm", color);
            }
            vdev_puts(vdev_ctx, esc);
        }
    }
    return err;
}
