/**
 * @file on_key.c
 * @brief ON KEY(n) GOSUB keyboard event trap statement handler for BASIC++.
 */
#include "statements/event/on_key.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_on_key_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ON KEY",
        .category = "Event Trapping",
        .syntax = "ON KEY(n) GOSUB line_label | KEY(n) {ON|OFF|STOP} | KEY ON | KEY OFF | KEY n, string",
        .help_text = "Establishes an asynchronous interrupt handler for function key presses, or toggles/customizes row 25 function key labels.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_on_key_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VConContext *vcon = vm_get_vcon(vm);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        char kw[32];
        if (tok.length >= sizeof(kw)) tok.length = sizeof(kw) - 1;
        memcpy(kw, tok.start, tok.length);
        kw[tok.length] = '\0';

        if (platform_strcasecmp(kw, "ON") == 0) {
            lex_next(lex);
            if (vcon) vcon_set_key_labels_visible(vcon, true);
            return err;
        }
        if (platform_strcasecmp(kw, "OFF") == 0) {
            lex_next(lex);
            if (vcon) vcon_set_key_labels_visible(vcon, false);
            return err;
        }
    }

    BValue val1 = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val1.type == VAL_NUMBER) {
        int key_idx = (int)val1.as.number;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue sval = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (sval.type == VAL_STRING && sval.as.string) {
                if (vcon) vcon_set_key_label(vcon, key_idx, str_data(sval.as.string));
                str_release(vm_get_str(vm), sval.as.string);
            }
        }
    }

    return err;
}
