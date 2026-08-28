// FILENAME: msgbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, msgbox.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the MSGBOX statement in BASIC++.
//
// ---- Includes ----

#include "statements/ui/widgets/msgbox.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <string.h>

BppError stmt_msgbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5;
        err.message = "Invalid context for MSGBOX";
        return err;
    }

    // 1. Evaluate prompt expression
    BValue val_prompt = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    char prompt_buf[1024] = {0};
    if (val_prompt.type == VAL_STRING) {
        const char *ps = str_data(val_prompt.as.string);
        if (ps) {
            strncpy(prompt_buf, ps, sizeof(prompt_buf) - 1);
        }
        str_release(vm_get_str(vm), val_prompt.as.string);
    } else if (val_prompt.type == VAL_NUMBER) {
        snprintf(prompt_buf, sizeof(prompt_buf), "%g", val_prompt.as.number);
    } else {
        err.code = 13;
        err.message = "Type mismatch in MSGBOX: prompt must be string or number";
        return err;
    }

    int buttons = 0;
    char title_buf[256] = "Visual Basic";

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // consume comma

        tok = lex_peek(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_EOL && tok.type != TOK_EOF) {
            BValue val_btn = eval_expression(vm, lex, &err);
            if (err.code == 0) {
                if (val_btn.type == VAL_NUMBER) {
                    buttons = (int)val_btn.as.number;
                } else if (val_btn.type == VAL_STRING) {
                    str_release(vm_get_str(vm), val_btn.as.string);
                }
            } else {
                return err;
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // consume second comma
            tok = lex_peek(lex);
            if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
                BValue val_title = eval_expression(vm, lex, &err);
                if (err.code == 0) {
                    if (val_title.type == VAL_STRING) {
                        const char *ts = str_data(val_title.as.string);
                        if (ts) {
                            strncpy(title_buf, ts, sizeof(title_buf) - 1);
                        }
                        str_release(vm_get_str(vm), val_title.as.string);
                    } else if (val_title.type == VAL_NUMBER) {
                        snprintf(title_buf, sizeof(title_buf), "%g", val_title.as.number);
                    }
                } else {
                    return err;
                }
            }
        }
    }

    (void)buttons;
    // Output formatted message box header & content to console virtual device
    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        char out[1536];
        snprintf(out, sizeof(out), "[ %s ]\n%s\n", title_buf, prompt_buf);
        vdev_puts(vdev_ctx, out);
    }

    return err;
}
