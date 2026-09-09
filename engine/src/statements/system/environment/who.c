// FILENAME: who.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, who.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the WHO statement in BASIC++.

#include "statements/system/environment/who.h"
#include "runtime/language_descriptor.h"
#include "runtime/session.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_who_desc = {
    .name = "WHO",
    .category = "Session & Multi-User",
    .syntax = "WHO",
    .description = "Displays active user session telemetry, job ID, TTY line, and priority status.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_who_register(void) {
    lang_desc_register(&g_who_desc);
}

BppError stmt_who_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHO) {
        lex_next(lex);
    }

    BppSessionContext *sess = vm_get_session(vm);
    char out[128];
    runtime_snprintf(out, sizeof(out), "Job %d: %s [%s] (TTY%d, Priority: %d, Echo: %s)\n",
             session_get_job_id(sess),
             session_get_username(sess),
             session_get_account(sess),
             session_get_tty_id(sess),
             session_get_priority(sess),
             session_get_echo(sess) ? "ON" : "OFF");
    vdev_puts(vm_get_vdev(vm), out);

    return err;
}
