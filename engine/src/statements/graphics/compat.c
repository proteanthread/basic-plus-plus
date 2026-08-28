// FILENAME: compat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (compat.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (bgi_gfx.h, bgi_gfx.c, errors.h)
// Provides runtime implementation for the COMPAT statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/compat.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "device/bgi_gfx.h"
#include <string.h>

extern BppError stmt_screen_handler(VMContext *vm, LexerContext *lex);

BppError stmt_gr_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); bgi_gfx_set_screen_mode_custom(40, 40, 4, 40, 4, 60.0f); return e; }
BppError stmt_hgr_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); bgi_gfx_set_screen_mode_custom(280, 160, 3, 40, 4, 60.0f); return e; }
BppError stmt_hgr2_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); bgi_gfx_set_screen_mode_custom(280, 192, 3, 0, 0, 60.0f); return e; }
BppError stmt_hcolor_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_hlin_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_vlin_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_hplot_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }

BppError stmt_graphics_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int mode_arg = 0;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING) {
            str_release(vm_get_str(vm), val.as.string);
            err.code = 13;
            err.message = "Type mismatch: GRAPHICS expects numeric mode";
            return err;
        }
        mode_arg = (int)val.as.number;
    }

    bool full_screen = (mode_arg & 16) != 0;
    int base_mode = mode_arg & 15;

    int w = 320, h = 192, bpp = 4;
    int text_cols = 40, text_rows = full_screen ? 0 : 4;

    switch (base_mode) {
        case 0:  w = 320; h = 192; bpp = 1; text_cols = 40; text_rows = 24; break;
        case 1:  w = 160; h = 160; bpp = 2; text_cols = 20; text_rows = 24; break;
        case 2:  w = 160; h = 80;  bpp = 2; text_cols = 20; text_rows = 12; break;
        case 3:  w = 40;  h = 24;  bpp = 2; break;
        case 4:  w = 80;  h = 48;  bpp = 1; break;
        case 5:  w = 80;  h = 48;  bpp = 2; break;
        case 6:  w = 160; h = 96;  bpp = 1; break;
        case 7:  w = 160; h = 96;  bpp = 2; break;
        case 8:  w = 320; h = 192; bpp = 1; break;
        case 9:  w = 80;  h = 192; bpp = 4; break;
        case 10: w = 80;  h = 192; bpp = 4; break;
        case 11: w = 80;  h = 192; bpp = 4; break;
        case 12: w = 80;  h = 192; bpp = 2; break;
        case 13: w = 160; h = 192; bpp = 2; break;
        case 14: w = 160; h = 192; bpp = 1; break;
        case 15: w = 320; h = 192; bpp = 2; break;
        default:
            err.code = 5;
            err.message = "Illegal Function Call: Invalid Atari GRAPHICS mode";
            return err;
    }

    bgi_gfx_set_screen_mode_custom(w, h, bpp, text_cols, text_rows, 60.0f);
    return err;
}

BppError stmt_mode_handler(VMContext *vm, LexerContext *lex) {
    return stmt_screen_handler(vm, lex);
}

BppError stmt_drawto_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_border_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_ink_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }
BppError stmt_paper_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError e; memset(&e, 0, sizeof(e)); return e; }

void stmt_gfx_compat_register(void) {
    MicroLibMetadata meta = {
        .name = "GFX.COMPAT",
        .category = "Graphics & Retro Modes",
        .syntax = "SET MODE mode_num",
        .help_text = "Sets retro graphics hardware compatibility profiles (CGA, EGA, VGA, Atari, Hercules).",
        .error_codes = "Error 5: Illegal Function Call (unsupported graphics mode)"
    };
    microlib_register(&meta);
}
