// FILENAME: screen.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, screen.h, string.c, vm.h)
// NEEDS: libkernel (bgi_autodetect.h, bgi_autodetect.c, bgi_gfx.h, bgi_gfx.c)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SCREEN statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/screen/screen.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "device/bgi_gfx.h"
#include "device/bgi_autodetect.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

extern BppError vdev_legacy_stmt_screen_handler(VMContext *vm, LexerContext *lex);
extern BppError vdev_legacy_stmt_screen_mode_handler(VMContext *vm, int mode);

void stmt_screen_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SCREEN",
        .category = "Graphics & Display",
        .syntax = "SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]]",
        .help_text = "Sets text (SCREEN 0), heritage (SCREEN 1-13), or modern high-res display mode (SCREEN 14-20: 800x600 to 4K).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

static BppError parse_custom_resolution(VMContext *vm, LexerContext *lex, int width, int default_bpp, int default_text_cols, int default_text_rows) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue val_h = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_h.type == VAL_STRING) {
        str_release(vm_get_str(vm), val_h.as.string);
        err.code = 13;
        err.message = "Type mismatch: height expects numeric value";
        return err;
    }
    int height = (int)val_h.as.number;

    int bpp = default_bpp;
    float fps = 60.0f;
    int text_cols = -1, text_rows = -1;
    bool tc_supplied = false, tr_supplied = false;

    // 3rd arg: bpp parameter
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue val_bpp = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_bpp.type != VAL_STRING) bpp = (int)val_bpp.as.number;
        else str_release(vm_get_str(vm), val_bpp.as.string);
    }
    // 4th arg: text_cols parameter
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue val_tc = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_tc.type != VAL_STRING) { text_cols = (int)val_tc.as.number; tc_supplied = true; }
        else str_release(vm_get_str(vm), val_tc.as.string);
    }
    // 5th arg: text_rows parameter
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue val_tr = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_tr.type != VAL_STRING) { text_rows = (int)val_tr.as.number; tr_supplied = true; }
        else str_release(vm_get_str(vm), val_tr.as.string);
    }
    // 6th arg: fps parameter
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
        BValue val_fps = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_fps.type != VAL_STRING) fps = (float)val_fps.as.number;
        else str_release(vm_get_str(vm), val_fps.as.string);
    }

    // Apply asymmetrical text window defaults
    if (tc_supplied && !tr_supplied) text_rows = (default_text_rows > 0) ? default_text_rows : 4;
    if (!tc_supplied && tr_supplied) text_cols = (default_text_cols > 0) ? default_text_cols : 80;
    if (!tc_supplied && !tr_supplied) { text_cols = default_text_cols; text_rows = default_text_rows; }

    bgi_gfx_set_screen_mode_custom(width, height, bpp, text_cols, text_rows, fps);
    return err;
}

BppError stmt_screen_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int first_arg = 0;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING) {
            str_release(vm_get_str(vm), val.as.string);
            err.code = 13;
            err.message = "Type mismatch: SCREEN expects numeric mode or width";
            return err;
        }
        first_arg = (int)val.as.number;

        // Check if comma follows (custom width, height initializer syntax)
        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); // consume comma
            return parse_custom_resolution(vm, lex, first_arg, 8, 0, 0);
        }

        // Single argument heritage SCREEN mode
        if (first_arg < 0 || first_arg > 20) {
            err.code = 5;
            err.message = "Illegal Function Call: SCREEN mode out of range (0-20)";
            return err;
        }
        bgi_gfx_set_screen_mode(first_arg);
        return vdev_legacy_stmt_screen_mode_handler(vm, first_arg);
    }

    bgi_gfx_set_screen_mode(0);
    return vdev_legacy_stmt_screen_mode_handler(vm, 0);
}

BppError stmt_title_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_resize_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_icon_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
