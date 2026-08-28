// FILENAME: key.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, key.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vcon.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the KEY statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/key.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>
#include <stdio.h>

void stmt_key_register(void) {
    static const MicroLibMetadata meta = {
        .name = "KEY",
        .category = "Event Trapping & Console",
        .syntax = "KEY ON | KEY OFF | KEY LIST | KEY n, string$ | KEY(n) {ON|OFF|STOP}",
        .help_text = "Enables, disables, or customizes function key softkey display and key event trapping.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_key_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    VConContext *vcon = vm_get_vcon(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    BppToken tok = lex_peek(lex);

    // 1. KEY ON / KEY OFF / KEY LIST
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) ||
            (tok.type == TOK_IDENT && tok.length == 2 && platform_strncasecmp(tok.start, "ON", 2) == 0)) {
            lex_next(lex);
            if (vcon) vcon_set_key_labels_visible(vcon, true);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
            (tok.type == TOK_IDENT && tok.length == 3 && platform_strncasecmp(tok.start, "OFF", 3) == 0)) {
            lex_next(lex);
            if (vcon) vcon_set_key_labels_visible(vcon, false);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_LIST) ||
            (tok.type == TOK_IDENT && tok.length == 4 && platform_strncasecmp(tok.start, "LIST", 4) == 0)) {
            lex_next(lex);
            for (int i = 1; i <= 10; i++) {
                const char *lbl = vcon ? vcon_get_key_label(vcon, i) : "";
                if (vdev) {
                    vdev_printf(vdev, "F%-2d %s\n", i, lbl ? lbl : "");
                }
            }
            return err;
        }
    }

    // 2. KEY(n) ON | OFF | STOP
    if (tok.type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BValue kval = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (kval.type != VAL_NUMBER) {
            if (kval.type == VAL_STRING && kval.as.string) {
                str_release(vm_get_str(vm), kval.as.string);
            }
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        int key_idx = (int)kval.as.number;
        if (key_idx < 1 || key_idx > 20) {
            err.code = ERR_ILLEGAL_FUNCTION_CALL;
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type != TOK_RPAREN) {
            err.code = ERR_SYNTAX;
            return err;
        }
        lex_next(lex); // Consume ')'

        tok = lex_peek(lex);
        int state = -1;
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) ||
            (tok.type == TOK_IDENT && tok.length == 2 && platform_strncasecmp(tok.start, "ON", 2) == 0)) {
            state = 1;
            lex_next(lex);
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
                   (tok.type == TOK_IDENT && tok.length == 3 && platform_strncasecmp(tok.start, "OFF", 3) == 0)) {
            state = 0;
            lex_next(lex);
        } else if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_STOP) ||
                   (tok.type == TOK_IDENT && tok.length == 4 && platform_strncasecmp(tok.start, "STOP", 4) == 0)) {
            state = 2;
            lex_next(lex);
        } else {
            err.code = ERR_SYNTAX;
            return err;
        }

        vm_set_key_state(vm, key_idx, state);
        return err;
    }

    // 3. KEY n, string$
    BValue nval = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (nval.type != VAL_NUMBER) {
        if (nval.type == VAL_STRING && nval.as.string) {
            str_release(vm_get_str(vm), nval.as.string);
        }
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int key_idx = (int)nval.as.number;
    if (key_idx < 1 || key_idx > 20) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        return err;
    }
    lex_next(lex); // Consume ','

    BValue sval = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (sval.type != VAL_STRING) {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    if (vcon && sval.as.string) {
        vcon_set_key_label(vcon, key_idx, str_data(sval.as.string));
    }
    if (sval.as.string) {
        str_release(vm_get_str(vm), sval.as.string);
    }

    return err;
}
