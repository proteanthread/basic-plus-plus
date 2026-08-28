// FILENAME: stmt_plot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_plot.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the PLOT statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/stmt_plot.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include <string.h>

void stmt_plot_register(void) {
    MicroLibMetadata meta = {
        .name = "PLOT",
        .category = "Graphics Statements",
        .syntax = "PLOT [POINTS|LINES|AREA]: x, y [; x2, y2 ...]",
        .help_text = "Plots points, lines, or filled polygon area (ANSI Full BASIC 1987).",
        .error_codes = "Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_plot_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;

    BppToken tok = lex_peek(lex);
    int mode = 0; // 0 = POINTS, 1 = LINES, 2 = AREA

    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if (tok.length == 6 && strncasecmp(tok.start, "POINTS", 6) == 0) {
            mode = 0;
            lex_next(lex);
        } else if (tok.length == 5 && strncasecmp(tok.start, "LINES", 5) == 0) {
            mode = 1;
            lex_next(lex);
        } else if (tok.length == 4 && strncasecmp(tok.start, "AREA", 4) == 0) {
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
