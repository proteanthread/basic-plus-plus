// FILENAME: stmt_home.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (color_map.h, eval.h, lexer.h, stmt_home.h, vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vdev.h)
// Provides runtime implementation for the HOME statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/stmt_home.h"
#include "device/color_map.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_home_desc = {
    .name = "HOME",
    .category = "Graphics & Display",
    .syntax = "HOME [[fg] [, bg]]",
    .description = "Moves cursor to top-left (1, 1) without clearing screen, optionally setting text colors.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const int g_ansi_fg_map[16] = {
    30, 34, 32, 36, 31, 35, 33, 37,
    90, 94, 92, 96, 91, 95, 93, 97
};

static const int g_ansi_bg_map[16] = {
    40, 44, 42, 46, 41, 45, 43, 47,
    100, 104, 102, 106, 101, 105, 103, 107
};

void stmt_home_register(void) {
    lang_desc_register(&g_home_desc);
}

BppError stmt_home_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    int fg = -1;
    int bg = -1;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // consume leading comma: HOME , bg
        if (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
            bg = color_parse_token_or_expr(vm, lex, &err);
            if (err.code != 0) return err;
        }
    } else if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        fg = color_parse_token_or_expr(vm, lex, &err);
        if (err.code != 0) return err;

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // consume comma
            if (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
                bg = color_parse_token_or_expr(vm, lex, &err);
                if (err.code != 0) return err;
            }
        }
    }

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);

    // Apply color settings and cursor position to Virtual Console (without clearing grid)
    if (vcon) {
        if (fg >= 0 || bg >= 0) {
            vcon_set_color(vcon, 0, (fg >= 0 && fg <= 15) ? fg : -1, (bg >= 0 && bg <= 15) ? bg : -1);
        }
        vcon_locate(vcon, 0, 1, 1);
    }

    // Apply ANSI sequences and cursor home to virtual device (without clearing screen)
    if (vdev) {
        if (fg >= 0 && fg <= 15) {
            vdev_printf(vdev, "\033[%dm", g_ansi_fg_map[fg]);
        } else if (fg > 15) {
            int r = (fg >> 16) & 0xFF, g = (fg >> 8) & 0xFF, b = fg & 0xFF;
            vdev_printf(vdev, "\033[38;2;%d;%d;%dm", r, g, b);
        }

        if (bg >= 0 && bg <= 15) {
            vdev_printf(vdev, "\033[%dm", g_ansi_bg_map[bg]);
        } else if (bg > 15) {
            int r = (bg >> 16) & 0xFF, g = (bg >> 8) & 0xFF, b = bg & 0xFF;
            vdev_printf(vdev, "\033[48;2;%d;%d;%dm", r, g, b);
        }

        vdev_printf(vdev, "\033[H");
    }

    return err;
}
