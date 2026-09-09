// FILENAME: ask.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h, variables.h, variables.c)
// NEEDS: libengine (ask.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the ASK statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/ask.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "device/vcon.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_ask_desc = {
    .name = "ASK",
    .category = "Environment & Graphics",
    .syntax = "ASK property variable [, ...]",
    .description = "ECMA-116 standard statement to query console, graphics, and environment attributes.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static bool parse_var_name(LexerContext *lex, char *out_name, size_t max_len) {
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT) return false;
    size_t len = (tok.length < max_len - 1) ? tok.length : max_len - 1;
    runtime_memcpy(out_name, tok.start, len);
    out_name[len] = '\0';
    return true;
}

static void skip_optional_comma(LexerContext *lex) {
    BppToken sep = lex_peek(lex);
    if (sep.type == TOK_COMMA) {
        lex_next(lex);
    }
}

BppError stmt_ask_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    int channel = 0;
    BppToken tok = lex_peek(lex);
    BppToken prop_tok;

    if (tok.type == TOK_HASH || tok.type == TOK_NUMBER) {
        if (tok.type == TOK_HASH) lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type == VAL_NUMBER) channel = (int)ch_val.as.number;

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_EOL || sep.type == TOK_COMMA) {
            lex_next(lex);
        }
        prop_tok = lex_next(lex);
    } else {
        prop_tok = lex_next(lex);
    }

    char prop_name[64] = {0};
    size_t plen = (prop_tok.length < sizeof(prop_name) - 1) ? prop_tok.length : sizeof(prop_name) - 1;
    runtime_memcpy(prop_name, prop_tok.start, plen);

    if (runtime_strcasecmp(prop_name, "MARGIN") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_MARGIN)) {
        char var_name[64] = {0};
        if (!parse_var_name(lex, var_name, sizeof(var_name))) {
            err.code = 2; err.message = "Expected variable name in ASK MARGIN";
            return err;
        }
        BValue val = { .type = VAL_NUMBER, .as.number = 80.0 };
        var_assign(vm_get_var(vm), var_name, val);
        return err;
    }

    if (runtime_strcasecmp(prop_name, "ZONEWIDTH") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_ZONE)) {
        char var_name[64] = {0};
        if (!parse_var_name(lex, var_name, sizeof(var_name))) {
            err.code = 2; err.message = "Expected variable name in ASK ZONEWIDTH";
            return err;
        }
        BValue val = { .type = VAL_NUMBER, .as.number = 14.0 };
        var_assign(vm_get_var(vm), var_name, val);
        return err;
    }

    if (runtime_strcasecmp(prop_name, "CURSOR") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_CURSOR)) {
        char r_name[64] = {0}, c_name[64] = {0};
        if (!parse_var_name(lex, r_name, sizeof(r_name))) {
            err.code = 2; err.message = "Expected row variable in ASK CURSOR";
            return err;
        }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, c_name, sizeof(c_name))) {
            err.code = 2; err.message = "Expected column variable in ASK CURSOR";
            return err;
        }

        int row = 0, col = 0;
        VConContext *vcon = vm_get_vcon(vm);
        if (vcon) {
            vcon_get_cursor(vcon, 0, &row, &col);
        }

        BValue r_val = { .type = VAL_NUMBER, .as.number = (double)(row + 1) };
        BValue c_val = { .type = VAL_NUMBER, .as.number = (double)(col + 1) };
        var_assign(vm_get_var(vm), r_name, r_val);
        var_assign(vm_get_var(vm), c_name, c_val);
        return err;
    }

    if (runtime_strcasecmp(prop_name, "COLOR") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_COLOR)) {
        char fg_name[64] = {0};
        if (!parse_var_name(lex, fg_name, sizeof(fg_name))) {
            err.code = 2; err.message = "Expected foreground variable in ASK COLOR";
            return err;
        }
        int fg = 7, bg = 0;
        VConContext *vcon = vm_get_vcon(vm);
        if (vcon) {
            int attr = vcon_get_attr_at(vcon, 0, 1, 1);
            if (attr >= 0) {
                fg = attr & 0x0F;
                bg = (attr >> 4) & 0x07;
            }
        }
        BValue fg_val = { .type = VAL_NUMBER, .as.number = (double)fg };
        var_assign(vm_get_var(vm), fg_name, fg_val);

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex);
            char bg_name[64] = {0};
            if (parse_var_name(lex, bg_name, sizeof(bg_name))) {
                BValue bg_val = { .type = VAL_NUMBER, .as.number = (double)bg };
                var_assign(vm_get_var(vm), bg_name, bg_val);
            }
        }
        return err;
    }

    if (runtime_strcasecmp(prop_name, "SCREEN") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_SCREEN)) {
        char r_name[64] = {0}, c_name[64] = {0};
        if (!parse_var_name(lex, r_name, sizeof(r_name))) {
            err.code = 2; err.message = "Expected rows variable in ASK SCREEN";
            return err;
        }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, c_name, sizeof(c_name))) {
            err.code = 2; err.message = "Expected columns variable in ASK SCREEN";
            return err;
        }

        int rows = VCON_ROWS, cols = VCON_COLS;
        BValue r_val = { .type = VAL_NUMBER, .as.number = (double)rows };
        BValue c_val = { .type = VAL_NUMBER, .as.number = (double)cols };
        var_assign(vm_get_var(vm), r_name, r_val);
        var_assign(vm_get_var(vm), c_name, c_val);
        return err;
    }

    if (runtime_strcasecmp(prop_name, "WINDOW") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_WINDOW)) {
        char x1_n[64] = {0}, x2_n[64] = {0}, y1_n[64] = {0}, y2_n[64] = {0};
        if (!parse_var_name(lex, x1_n, sizeof(x1_n))) { err.code = 2; err.message = "Expected x1 in ASK WINDOW"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, y1_n, sizeof(y1_n))) { err.code = 2; err.message = "Expected y1 in ASK WINDOW"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, x2_n, sizeof(x2_n))) { err.code = 2; err.message = "Expected x2 in ASK WINDOW"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, y2_n, sizeof(y2_n))) { err.code = 2; err.message = "Expected y2 in ASK WINDOW"; return err; }

        double x1 = 0.0, y1 = 0.0, x2 = 639.0, y2 = 479.0;
        BValue v_x1 = { .type = VAL_NUMBER, .as.number = x1 };
        BValue v_y1 = { .type = VAL_NUMBER, .as.number = y1 };
        BValue v_x2 = { .type = VAL_NUMBER, .as.number = x2 };
        BValue v_y2 = { .type = VAL_NUMBER, .as.number = y2 };
        var_assign(vm_get_var(vm), x1_n, v_x1);
        var_assign(vm_get_var(vm), y1_n, v_y1);
        var_assign(vm_get_var(vm), x2_n, v_x2);
        var_assign(vm_get_var(vm), y2_n, v_y2);
        return err;
    }

    if (runtime_strcasecmp(prop_name, "VIEWPORT") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_VIEWPORT)) {
        char x1_n[64] = {0}, x2_n[64] = {0}, y1_n[64] = {0}, y2_n[64] = {0};
        if (!parse_var_name(lex, x1_n, sizeof(x1_n))) { err.code = 2; err.message = "Expected x1 in ASK VIEWPORT"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, y1_n, sizeof(y1_n))) { err.code = 2; err.message = "Expected y1 in ASK VIEWPORT"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, x2_n, sizeof(x2_n))) { err.code = 2; err.message = "Expected x2 in ASK VIEWPORT"; return err; }
        skip_optional_comma(lex);
        if (!parse_var_name(lex, y2_n, sizeof(y2_n))) { err.code = 2; err.message = "Expected y2 in ASK VIEWPORT"; return err; }

        double x1 = 0.0, y1 = 0.0, x2 = 1.0, y2 = 1.0;
        BValue v_x1 = { .type = VAL_NUMBER, .as.number = x1 };
        BValue v_y1 = { .type = VAL_NUMBER, .as.number = y1 };
        BValue v_x2 = { .type = VAL_NUMBER, .as.number = x2 };
        BValue v_y2 = { .type = VAL_NUMBER, .as.number = y2 };
        var_assign(vm_get_var(vm), x1_n, v_x1);
        var_assign(vm_get_var(vm), y1_n, v_y1);
        var_assign(vm_get_var(vm), x2_n, v_x2);
        var_assign(vm_get_var(vm), y2_n, v_y2);
        return err;
    }

    BppToken var_tok = lex_next(lex);
    if (var_tok.type != TOK_IDENT && var_tok.type != TOK_NUMBER) {
        err.code = 2; err.message = "Expected target variable in ASK statement";
        return err;
    }

    if (var_tok.type == TOK_IDENT) {
        char var_name[64] = {0};
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, var_tok.start, len);
        BValue val = { .type = VAL_NUMBER, .as.number = (double)channel };
        var_assign(vm_get_var(vm), var_name, val);
    }
    return err;
}

void stmt_ask_register(void) {
    lang_desc_register(&g_ask_desc);
}
