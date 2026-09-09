// FILENAME: login.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h, strings.h)
// NEEDS: libengine (eval.h, login.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the LOGIN / HELLO statement in BASIC++.

#include "statements/system/environment/login.h"
#include "runtime/language_descriptor.h"
#include "runtime/session.h"
#include "runtime/strings.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_login_desc = {
    .name = "LOGIN",
    .category = "Session & Multi-User",
    .syntax = "LOGIN username$ [, account$] | HELLO username$ [, account$]",
    .description = "Logs in a timesharing user session with optional account code.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_login_register(void) {
    lang_desc_register(&g_login_desc);
}

BppError stmt_login_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_LOGIN || tok.as.keyword == KW_HELLO)) {
        lex_next(lex);
    }

    char username[64] = "USER";
    char account[64] = "1,1";

    BppToken next = lex_peek(lex);
    if (next.type != TOK_EOL && next.type != TOK_EOF) {
        BValue uval = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (uval.type == VAL_STRING && uval.as.string) {
            runtime_strncpy(username, str_data(uval.as.string), sizeof(username) - 1);
            str_release(vm_get_str(vm), uval.as.string);
        }

        BppToken comma = lex_peek(lex);
        if (comma.type == TOK_COMMA) {
            lex_next(lex);
            BValue aval = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (aval.type == VAL_STRING && aval.as.string) {
                runtime_strncpy(account, str_data(aval.as.string), sizeof(account) - 1);
                str_release(vm_get_str(vm), aval.as.string);
            }
        }
    }

    BppSessionContext *sess = vm_get_session(vm);
    session_login(sess, username, account);

    char out[128];
    runtime_snprintf(out, sizeof(out), "Logged in as %s [%s] on TTY%d\n", session_get_username(sess), session_get_account(sess), session_get_tty_id(sess));
    vdev_puts(vm_get_vdev(vm), out);

    return err;
}
