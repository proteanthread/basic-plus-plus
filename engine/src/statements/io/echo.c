// FILENAME: echo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (session.h, session.c, string.h)
// NEEDS: libengine (echo.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the ECHO statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/echo.h"
#include "runtime/session.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "device/vdev.h"
#include <string.h>

void stmt_echo_register(void) {
    static const MicroLibMetadata meta_echo = {
        .name = "ECHO",
        .category = "Console & Keyboard",
        .syntax = "ECHO [ON | OFF]",
        .help_text = "Enables terminal input character echo (DEC / Timesharing).",
        .error_codes = "None"
    };
    microlib_register(&meta_echo);

    static const MicroLibMetadata meta_noecho = {
        .name = "NOECHO",
        .category = "Console & Keyboard",
        .syntax = "NO ECHO | NOECHO",
        .help_text = "Disables terminal input character echo for secure/password entry.",
        .error_codes = "None"
    };
    microlib_register(&meta_noecho);
}

BppError stmt_echo_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ECHO) {
        lex_next(lex);
    }

    bool echo = true;
    BppToken next = lex_peek(lex);
    if (next.type == TOK_KEYWORD && next.as.keyword == KW_OFF) {
        lex_next(lex);
        echo = false;
    } else if (next.type == TOK_KEYWORD && next.as.keyword == KW_ON) {
        lex_next(lex);
        echo = true;
    }

    BppSessionContext *sess = vm_get_session(vm);
    if (sess) {
        session_set_echo(sess, echo);
    }
    return err;
}

BppError stmt_noecho_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_NOECHO) {
        lex_next(lex);
    } else if (tok.type == TOK_IDENT && tok.length == 2 && ((tok.start[0] == 'n' || tok.start[0] == 'N') && (tok.start[1] == 'o' || tok.start[1] == 'O'))) {
        lex_next(lex);
        BppToken echo_tok = lex_peek(lex);
        if (echo_tok.type == TOK_KEYWORD && echo_tok.as.keyword == KW_ECHO) {
            lex_next(lex);
        }
    }

    BppSessionContext *sess = vm_get_session(vm);
    if (sess) {
        session_set_echo(sess, false);
    }
    return err;
}
