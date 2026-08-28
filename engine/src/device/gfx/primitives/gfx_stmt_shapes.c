// FILENAME: gfx_stmt_shapes.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (bgi.h, bgi.c)
// NEEDS: libkernel (gfx_primitives_internal.h)
// Implements virtual device and graphics rendering logic for gfx_stmt_shapes.
//
// ---- Includes ----

#include "device/gfx_primitives_internal.h"
#include "device/bgi.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

//
// ---- Geometric Shape Statements ----

BppError vdev_legacy_stmt_line_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) {
        lex_next(lex);
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
            runtime_memcpy(flag, tok.start, flen);
            for (size_t i = 0; i < flen; i++) flag[i] = (char)runtime_toupper((unsigned char)flag[i]);

            if (runtime_strcmp(flag, "B") == 0) {
                box = true;
            } else if (runtime_strcmp(flag, "BF") == 0) {
                box = true;
                box_fill = true;
            }
        }
    }

    int c_idx = (color >= 0 && color < 256) ? color : g_fg_color_idx;
    int x1 = (int)x1_val.as.number;
    int y1 = (int)y1_val.as.number;
    int x2 = (int)x2_val.as.number;
    int y2 = (int)y2_val.as.number;

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

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
        bgi_sync_screen_mode(graphics_mode);
        BGI_present(bgi);
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
}

BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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

    int c_idx = (color >= 0 && color < 256) ? color : g_fg_color_idx;
    int cx = (int)cx_val.as.number;
    int cy = (int)cy_val.as.number;
    int r = (int)rad_val.as.number;

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    if (bgi->initialized && bgi->framebuffer) {
        BGI_setcolor(bgi, c_idx);
        BGI_circle(bgi, cx, cy, r);
        bgi_sync_screen_mode(graphics_mode);
        BGI_present(bgi);
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
}

BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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

    int c_idx = (color >= 0 && color < 256) ? color : g_fg_color_idx;
    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    if (bgi->initialized && bgi->framebuffer) {
        BGI_putpixel(bgi, x, y, c_idx);
        bgi_sync_screen_mode(graphics_mode);
        BGI_present(bgi);
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
}

BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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

    int x = (int)x_val.as.number;
    int y = (int)y_val.as.number;

    BGI_Context *bgi = BGI_get_global_context();
    if (!bgi->initialized) {
        BGI_init(bgi);
    }

    if (bgi->initialized && bgi->framebuffer) {
        BGI_putpixel(bgi, x, y, bgi->bg_color);
        bgi_sync_screen_mode(graphics_mode);
        BGI_present(bgi);
    }

    VDev *con = vdev_get(vm_get_vdev(vm), "CON:");
    if (con) gfx_con_flush(con);

    return err;
}

BppError vdev_legacy_stmt_cls_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BGI_Context *bgi = BGI_get_global_context();
    if (bgi && bgi->initialized && bgi->framebuffer) {
        BGI_clearviewport(bgi);
        BGI_present(bgi);
    }

    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        vdev_puts(vdev_ctx, "\033[2J\033[H");
    }

    return err;
}
