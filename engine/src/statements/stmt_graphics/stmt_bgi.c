/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_bgi.c
 * @brief BGI BASIC Statement Handlers — INITGRAPH, CLOSEGRAPH, PUTPIXEL, etc.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements BASIC statement handlers that expose the BGI
 *   graphics interface to BASIC++ programs. Each handler parses arguments
 *   from the lexer and delegates to the BGI C API.
 * - Why it exists: Bridges the BASIC++ VM statement system with the modular
 *   BGI graphics engine, allowing programs to use BGI drawing commands.
 * - Why it works this way: Each handler follows the standard BppError
 *   stmt_xxx_handler(VMContext*, LexerContext*) signature. Arguments are
 *   parsed via eval_expression(). Drawing calls go through BGI_* functions.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add new BGI-related statement handlers.
 * - What cannot be changed: The handler function signature pattern.
 * - What to expect: BGI context must be initialized before these are called.
 * - What to do if something breaks: Verify BGI_init() was called during boot.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: VMContext and LexerContext are valid. eval_expression works.
 * - Portability concerns: Pure ISO C17. No OS dependencies.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new stmt_xxx_handler functions.
 * - How to write external extensions: Follow the handler pattern.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "vm/vm.h"
#include "device/bgi.h"
#include "runtime/strings.h"
#include <string.h>

/* ======================================================================
 * INITGRAPH mode_id
 * Activates a BGI graphics mode by heritage mode ID.
 * Example: INITGRAPH 113  (VGA Mode 13h)
 * ====================================================================== */
BppError stmt_initgraph_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;

    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int mode_id = (int)mode_val.as.number;
    BGI_Context *ctx = BGI_get_global_context();

    if (!ctx->initialized) {
        BGI_init(ctx);
    }

    int result = BGI_set_mode_by_id(ctx, (uint32_t)mode_id);
    if (result < 0) {
        err.code = 5;
        err.message = "INITGRAPH: Invalid or unsupported mode ID";
    }

    return err;
}

/* ======================================================================
 * CLOSEGRAPH
 * Shuts down the BGI graphics subsystem and releases VRAM.
 * ====================================================================== */
BppError stmt_closegraph_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;
    (void)lex;

    BGI_Context *ctx = BGI_get_global_context();
    BGI_shutdown(ctx);

    return err;
}

/* ======================================================================
 * PUTPIXEL x, y, color
 * Writes a single pixel at (x, y) with the specified color.
 * ====================================================================== */
BppError stmt_putpixel_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected comma after X";
        return err;
    }
    lex_next(lex);

    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected comma after Y";
        return err;
    }
    lex_next(lex);

    BValue c_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) {
        err.code = 5;
        err.message = "PUTPIXEL: Graphics mode not initialized (use INITGRAPH first)";
        return err;
    }

    BGI_putpixel(ctx, (int)x_val.as.number, (int)y_val.as.number,
                 (int)c_val.as.number);

    return err;
}

/* ======================================================================
 * BAR x1, y1, x2, y2
 * Draws a filled rectangle using the current fill color/style.
 * ====================================================================== */
BppError stmt_bar_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue x1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
    lex_next(lex);

    BValue y1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
    lex_next(lex);

    BValue x2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
    lex_next(lex);

    BValue y2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) {
        err.code = 5;
        err.message = "BAR: Graphics mode not initialized";
        return err;
    }

    BGI_bar(ctx, (int)x1_val.as.number, (int)y1_val.as.number,
                 (int)x2_val.as.number, (int)y2_val.as.number);

    return err;
}

/* ======================================================================
 * ELLIPSE cx, cy, start_angle, end_angle, xradius, yradius
 * ====================================================================== */
BppError stmt_ellipse_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    double args[6];
    for (int i = 0; i < 6; ++i) {
        if (i > 0) {
            BppToken tok = lex_peek(lex);
            if (tok.type != TOK_COMMA) {
                err.code = 2;
                err.message = "ELLIPSE: Expected 6 comma-separated arguments";
                return err;
            }
            lex_next(lex);
        }
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        args[i] = val.as.number;
    }

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) {
        err.code = 5;
        err.message = "ELLIPSE: Graphics mode not initialized";
        return err;
    }

    BGI_ellipse(ctx, (int)args[0], (int)args[1], (int)args[2],
                     (int)args[3], (int)args[4], (int)args[5]);

    return err;
}

/* ======================================================================
 * RECTANGLE x1, y1, x2, y2
 * Draws an unfilled rectangle outline.
 * ====================================================================== */
BppError stmt_rectangle_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    double args[4];
    for (int i = 0; i < 4; ++i) {
        if (i > 0) {
            BppToken tok = lex_peek(lex);
            if (tok.type != TOK_COMMA) {
                err.code = 2;
                err.message = "RECTANGLE: Expected 4 comma-separated arguments";
                return err;
            }
            lex_next(lex);
        }
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        args[i] = val.as.number;
    }

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) {
        err.code = 5;
        err.message = "RECTANGLE: Graphics mode not initialized";
        return err;
    }

    BGI_rectangle(ctx, (int)args[0], (int)args[1],
                       (int)args[2], (int)args[3]);

    return err;
}

/* ======================================================================
 * OUTTEXTXY x, y, text$
 * Renders text at (x, y) using the active font and color.
 * ====================================================================== */
BppError stmt_outtextxy_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue x_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
    lex_next(lex);

    BValue y_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected ','"; return err; }
    lex_next(lex);

    BValue text_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized || !ctx->framebuffer) {
        err.code = 5;
        err.message = "OUTTEXTXY: Graphics mode not initialized";
        return err;
    }

    /* Get the string value */
    if (text_val.type == VAL_STRING && text_val.as.string) {
        const char *text_str = str_data(text_val.as.string);
        if (text_str) {
            BGI_outtextxy(ctx, (int)x_val.as.number, (int)y_val.as.number,
                          text_str);
        }
    }

    return err;
}

/* ======================================================================
 * PALETTE index, r, g, b
 * Sets a palette entry. The color is specified as (index, R, G, B).
 * ====================================================================== */
BppError stmt_palette_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BGI_Context *ctx = BGI_get_global_context();
    if (!ctx->initialized) {
        err.code = 5;
        err.message = "PALETTE: BGI not initialized";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
        /* No args: restore default palette */
        for (int i = 0; i < 16; i++) {
            uint8_t r = ((i & 4) ? 170 : 0) + ((i & 8) ? 85 : 0);
            uint8_t g = ((i & 2) ? 170 : 0) + ((i & 8) ? 85 : 0);
            uint8_t b = ((i & 1) ? 170 : 0) + ((i & 8) ? 85 : 0);
            if (i == 6) { r = 170; g = 85; b = 0; } /* Brown fix */
            uint32_t argb = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            BGI_setpalette(ctx, i, argb);
        }
        return err;
    }

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        BppToken name_tok = lex_next(lex);
        if (name_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected array name after PALETTE USING";
            return err;
        }
        char arr_name[256];
        size_t clen = (name_tok.length < 255) ? name_tok.length : 255;
        memcpy(arr_name, name_tok.start, clen);
        arr_name[clen] = '\0';

        /* check for optional parenthesis */
        int start_idx = 0;
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex); /* Consume '(' */
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            start_idx = (int)idx_val.as.number;
            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')'";
                return err;
            }
        }

        int total_size = 0;
        BValue *elems = arr_get_flat_elements(vm_get_arr(vm), arr_name, &total_size);
        if (!elems) {
            err.code = 9; err.message = "Array not found";
            return err;
        }

        for (int i = 0; i < 16; i++) {
            int arr_pos = start_idx + i;
            if (arr_pos >= 0 && arr_pos < total_size) {
                int code = (int)elems[arr_pos].as.number;
                if (code >= 0 && code <= 63) {
                    uint8_t r = ((code & 4) ? 170 : 0) + ((code & 32) ? 85 : 0);
                    uint8_t g = ((code & 2) ? 170 : 0) + ((code & 16) ? 85 : 0);
                    uint8_t b = ((code & 1) ? 170 : 0) + ((code & 8) ? 85 : 0);
                    if (code == 6) { r = 170; g = 85; b = 0; } /* Brown */
                    uint32_t argb = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
                    BGI_setpalette(ctx, i, argb);
                }
            }
        }
        return err;
    }

    /* Standard color mapping or custom RGB mapping */
    BValue val_idx = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int index = (int)val_idx.as.number;

    if (lex_peek(lex).type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' after palette index";
        return err;
    }
    lex_next(lex); /* Consume ',' */

    BValue val_color = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_COMMA) {
        /* Custom RGB syntax: PALETTE index, R, G, B */
        lex_next(lex); /* Consume ',' */
        BValue val_g = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (lex_next(lex).type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ','"; return err;
        }
        BValue val_b = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        uint8_t r = (uint8_t)(int)val_color.as.number;
        uint8_t g = (uint8_t)(int)val_g.as.number;
        uint8_t b = (uint8_t)(int)val_b.as.number;
        uint32_t argb = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        BGI_setpalette(ctx, index, argb);
    } else {
        /* Classic syntax: PALETTE index, color_code */
        int code = (int)val_color.as.number;
        if (code >= 0 && code <= 63) {
            uint8_t r = ((code & 4) ? 170 : 0) + ((code & 32) ? 85 : 0);
            uint8_t g = ((code & 2) ? 170 : 0) + ((code & 16) ? 85 : 0);
            uint8_t b = ((code & 1) ? 170 : 0) + ((code & 8) ? 85 : 0);
            if (code == 6) { r = 170; g = 85; b = 0; } /* Brown */
            uint32_t argb = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            BGI_setpalette(ctx, index, argb);
        }
    }

    return err;
}
