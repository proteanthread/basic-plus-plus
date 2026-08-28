// FILENAME: session_stmts.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (session.h, session.c, string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, session_stmts.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SESSION_STMTS statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/session_stmts.h"
#include "runtime/session.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <stdio.h>
#include <string.h>

void stmt_session_register(void) {
    static const MicroLibMetadata meta_login = {
        .name = "LOGIN",
        .category = "Session & Multi-User",
        .syntax = "LOGIN username$ [, account$] | HELLO username$ [, account$]",
        .help_text = "Logs in a timesharing user session with optional account code.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta_login);

    static const MicroLibMetadata meta_who = {
        .name = "WHO",
        .category = "Session & Multi-User",
        .syntax = "WHO",
        .help_text = "Displays active user session telemetry, job ID, TTY line, and priority status.",
        .error_codes = "None"
    };
    microlib_register(&meta_who);

    static const MicroLibMetadata meta_tty = {
        .name = "TTY",
        .category = "Session & Multi-User",
        .syntax = "TTY [tty_number%]",
        .help_text = "Displays or sets the current virtual terminal/teleprinter channel number.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta_tty);

    static const MicroLibMetadata meta_prio = {
        .name = "PRIORITY",
        .category = "Session & Multi-User",
        .syntax = "PRIORITY priority_level%",
        .help_text = "Sets timesharing CPU job execution scheduling priority (1..100).",
        .error_codes = "Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta_prio);
}

BppError stmt_login_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

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
            strncpy(username, str_data(uval.as.string), sizeof(username) - 1);
            str_release(vm_get_str(vm), uval.as.string);
        }

        BppToken comma = lex_peek(lex);
        if (comma.type == TOK_COMMA) {
            lex_next(lex);
            BValue aval = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (aval.type == VAL_STRING && aval.as.string) {
                strncpy(account, str_data(aval.as.string), sizeof(account) - 1);
                str_release(vm_get_str(vm), aval.as.string);
            }
        }
    }

    BppSessionContext *sess = vm_get_session(vm);
    session_login(sess, username, account);

    char out[128];
    snprintf(out, sizeof(out), "Logged in as %s [%s] on TTY%d\n", session_get_username(sess), session_get_account(sess), session_get_tty_id(sess));
    vdev_puts(vm_get_vdev(vm), out);

    return err;
}

BppError stmt_who_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHO) {
        lex_next(lex);
    }

    BppSessionContext *sess = vm_get_session(vm);
    char out[128];
    snprintf(out, sizeof(out), "Job %d: %s [%s] (TTY%d, Priority: %d, Echo: %s)\n",
             session_get_job_id(sess),
             session_get_username(sess),
             session_get_account(sess),
             session_get_tty_id(sess),
             session_get_priority(sess),
             session_get_echo(sess) ? "ON" : "OFF");
    vdev_puts(vm_get_vdev(vm), out);

    return err;
}

BppError stmt_tty_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

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
        snprintf(out, sizeof(out), "TTY%d\n", session_get_tty_id(sess));
        vdev_puts(vm_get_vdev(vm), out);
    }

    return err;
}

BppError stmt_priority_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PRIORITY) {
        lex_next(lex);
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_NUMBER) {
        err.code = 13; err.message = "PRIORITY expects numeric level";
        return err;
    }

    int p = (int)val.as.number;
    if (p < 1 || p > 100) {
        err.code = 5; err.message = "Priority level must be between 1 and 100";
        return err;
    }

    BppSessionContext *sess = vm_get_session(vm);
    session_set_priority(sess, p);
    return err;
}
