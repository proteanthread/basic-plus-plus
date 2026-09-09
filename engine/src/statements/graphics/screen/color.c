// FILENAME: color.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (color.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the COLOR statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/screen/color.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "statements/variables/data/pick_locate.h"

static const LangDesc g_color_desc = {
    .name = "COLOR",
    .category = "Graphics & Display",
    .syntax = "COLOR [foreground] [, [background] [, border]]",
    .description = "Sets active foreground, background, and border display colors.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_color_register(void) {
    lang_desc_register(&g_color_desc);
}

static const int ansi_fg_map[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};
static const int ansi_bg_map[16] = {40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107};

BppError stmt_color_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    int fg = -1, bg = -1, border = -1;

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_COMMA) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) fg = (int)val.as.number;
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_COMMA) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER) bg = (int)val.as.number;
            tok = lex_peek(lex);
        }
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER) border = (int)val.as.number;
        }
    }

    if (fg < -1 || fg > 31 || bg < -1 || bg > 15 || border < -1 || border > 15) {
        err.code = 5;
        err.message = "Illegal function call";
        return err;
    }

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    if (vcon) {
        vcon_set_color(vcon, 0, fg, bg);
    }

    if (vdev) {
        if (fg >= 0 && fg <= 15) {
            vdev_printf(vdev, "\033[%dm", ansi_fg_map[fg]);
        }
        if (bg >= 0 && bg <= 15) {
            vdev_printf(vdev, "\033[%dm", ansi_bg_map[bg]);
        }
    }
    (void)border;

    return err;
}

BppError stmt_bcolor_handler(VMContext *vm, LexerContext *lex) {
    return stmt_color_handler(vm, lex);
}

BppError stmt_fcolor_handler(VMContext *vm, LexerContext *lex) {
    return stmt_color_handler(vm, lex);
}

BppError stmt_cursor_handler(VMContext *vm, LexerContext *lex) {
    return stmt_locate_handler(vm, lex);
}

BppError stmt_locate_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    int row = -1, col = -1, vis = -1;

    const char *in_pos = NULL;
    if (pick_locate_has_in_clause(lex, &in_pos)) {
        size_t tlen = (size_t)(in_pos - lex_get_pos(lex));
        char *tbuf = (char *)mem_scratch_alloc(vm_get_mem(vm), tlen + 1);
        if (tbuf) {
            runtime_memcpy(tbuf, lex_get_pos(lex), tlen);
            tbuf[tlen] = '\0';
            LexerContext *sub = lex_init(vm_get_mem(vm), tbuf);
            BValue target_val = eval_expression(vm, sub, &err);
            lex_shutdown(sub);
            if (err.code != 0) return err;
            lex_set_pos(lex, in_pos);
            return stmt_pick_locate_execute(vm, lex, target_val);
        }
    }

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_COMMA) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        tok = lex_peek(lex);
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_IN) ||
            (tok.type == TOK_IDENT && tok.length == 2 && runtime_strncasecmp(tok.start, "IN", 2) == 0)) {
            return stmt_pick_locate_execute(vm, lex, val);
        }
        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            row = (int)val.as.number;
        } else if (val.type == VAL_STRING) {
            if (val.as.string) str_release(vm_get_str(vm), val.as.string);
        }
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_COMMA) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER) col = (int)val.as.number;
            tok = lex_peek(lex);
        }
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER) vis = (int)val.as.number;
        }
    }

    if ((row != -1 && (row < 1 || row > 255)) ||
        (col != -1 && (col < 1 || col > 255)) ||
        (vis != -1 && (vis < 0 || vis > 1))) {
        err.code = 5;
        err.message = "Illegal function call";
        return err;
    }

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    if (vcon && row > 0 && col > 0) {
        vcon_locate(vcon, 0, row, col);
    }

    if (vdev) {
        if (row > 0 && col > 0) {
            vdev_printf(vdev, "\033[%d;%dH", row, col);
        }
        if (vis == 0) {
            vdev_printf(vdev, "\033[?25l");
        } else if (vis == 1) {
            vdev_printf(vdev, "\033[?25h");
        }
    }

    return err;
}
