// FILENAME: tty.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, tty.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TTY statement in BASIC++.

#include "statements/system/environment/tty.h"
#include "runtime/language_descriptor.h"
#include "runtime/session.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_tty_desc = {
    .name = "TTY",
    .category = "Session & Multi-User",
    .syntax = "TTY [tty_number%]",
    .description = "Displays or sets the current virtual terminal/teleprinter channel number.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_tty_register(void) {
    lang_desc_register(&g_tty_desc);
}

BppError stmt_tty_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TTY) {
        lex_next(lex);
    }

    BppSessionContext *sess = vm_get_session(vm);
    BppToken next = lex_peek(lex);
    if (next.type != TOK_EOL && next.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            sess->tty_id = (int)val.as.number;
        }
    } else {
        char out[64];
        runtime_snprintf(out, sizeof(out), "TTY%d\n", session_get_tty_id(sess));
        vdev_puts(vm_get_vdev(vm), out);
    }

    return err;
}
