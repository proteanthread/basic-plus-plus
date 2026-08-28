// FILENAME: on_key.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_key.h, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ON_KEY statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/on_key.h"
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
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BValue kval = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (kval.type != VAL_NUMBER && kval.type != VAL_INTEGER) {
            if (kval.type == VAL_STRING && kval.as.string) {
                str_release(vm_get_str(vm), kval.as.string);
            }
            err.code = 13; err.message = "Type mismatch in ON KEY(n)";
            return err;
        }
        int key_idx = (int)kval.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in ON KEY(n)";
            return err;
        }

        tok = lex_next(lex);
        bool is_gosub = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOSUB) ||
                        (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "GOSUB", 5) == 0);
        bool is_goto  = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) ||
                        (tok.type == TOK_IDENT && tok.length == 4 && platform_strncasecmp(tok.start, "GOTO", 4) == 0);
        if (!is_gosub && !is_goto) {
            err.code = 2; err.message = "Expected GOSUB in ON KEY(n)";
            return err;
        }

        BValue line_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (line_val.type != VAL_NUMBER && line_val.type != VAL_INTEGER) {
            if (line_val.type == VAL_STRING && line_val.as.string) {
                str_release(vm_get_str(vm), line_val.as.string);
            }
            err.code = 13; err.message = "Type mismatch in target line";
            return err;
        }

        BppLineNumber target_line = (BppLineNumber)line_val.as.number;
        vm_set_key_trap(vm, key_idx, 0, target_line);
        return err;
    }

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
