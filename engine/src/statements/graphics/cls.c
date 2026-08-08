/**
 * @file cls.c
 * @brief CLS [0 | 1 | 2] screen clear statement handler implementation for BASIC++.
 */
#include "statements/graphics/cls.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

void stmt_cls_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CLS",
        .category = "Graphics & Display",
        .syntax = "CLS [n]",
        .help_text = "Clears display: CLS -1 (text viewport), CLS -2 (graphics viewport), CLS -3 (full buffer, default), or CLS n (0-15 background color clear).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_cls_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int mode = -3;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            mode = (int)val.as.number;
            if (mode < -3 || mode > 15) {
                err.code = 5;
                err.message = "Illegal Function Call";
                return err;
            }
        } else {
            err.code = 5;
            err.message = "Illegal Function Call";
            return err;
        }
    }

    VConContext *vcon = vm_get_vcon(vm);
    if (0 <= mode && mode <= 15) {
        if (vcon) {
            vcon_set_color(vcon, 0, -1, mode);
            vcon_clear_screen(vcon, 0, mode);
        }
        int sgr = (mode < 8) ? (40 + mode) : (100 + (mode - 8));
        printf("\033[%dm\033[2J\033[H", sgr);
        fflush(stdout);
    } else {
        if (vcon) {
            vcon_clear_screen(vcon, 0, mode);
        }
        printf("\033[2J\033[H");
        fflush(stdout);
    }

    return err;
}
