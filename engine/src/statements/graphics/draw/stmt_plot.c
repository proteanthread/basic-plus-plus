// FILENAME: stmt_plot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_plot.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the PLOT statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/stmt_plot.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_plot_desc = {
    .name = "PLOT",
    .category = "Graphics Statements",
    .syntax = "PLOT [POINTS|LINES|AREA]: x, y [; x2, y2 ...]",
    .description = "Plots points, lines, or filled polygon area (ANSI Full BASIC 1987).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_plot_register(void) {
    lang_desc_register(&g_plot_desc);
}

BppError stmt_plot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm;

    BppToken tok = lex_peek(lex);
    int mode = 0; // 0 = POINTS, 1 = LINES, 2 = AREA

    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if (tok.length == 6 && runtime_strncasecmp(tok.start, "POINTS", 6) == 0) {
            mode = 0;
            lex_next(lex);
        } else if (tok.length == 5 && runtime_strncasecmp(tok.start, "LINES", 5) == 0) {
            mode = 1;
            lex_next(lex);
        } else if (tok.length == 4 && runtime_strncasecmp(tok.start, "AREA", 4) == 0) {
            mode = 2;
            lex_next(lex);
        }
    }

    BppToken tok_next = lex_peek(lex);
    if ((tok_next.type == TOK_EOL || tok_next.type == TOK_UNKNOWN) && tok_next.length == 1 && tok_next.start && tok_next.start[0] == ':') {
        lex_next(lex);
    }

    // Parse coordinate pairs separated by comma/semicolon
    while (1) {
        BValue val_x = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_x.type == VAL_STRING) {
            str_release(vm_get_str(vm), val_x.as.string);
            err.code = 13;
            err.message = "PLOT expects numeric x coordinate";
            return err;
        }

        tok_next = lex_peek(lex);
        if (tok_next.type == TOK_COMMA) {
            lex_next(lex);
        }

        BValue val_y = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val_y.type == VAL_STRING) {
            str_release(vm_get_str(vm), val_y.as.string);
            err.code = 13;
            err.message = "PLOT expects numeric y coordinate";
            return err;
        }

        (void)mode;
        (void)val_x;
        (void)val_y;

        tok_next = lex_peek(lex);
        if (tok_next.type == TOK_SEMICOLON || tok_next.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}
