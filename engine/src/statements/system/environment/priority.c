// FILENAME: priority.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, priority.h)
// Provides runtime implementation for the PRIORITY statement in BASIC++.

#include "statements/system/environment/priority.h"
#include "runtime/language_descriptor.h"
#include "runtime/session.h"
#include "eval/eval.h"

static const LangDesc g_priority_desc = {
    .name = "PRIORITY",
    .category = "Session & Multi-User",
    .syntax = "PRIORITY priority_level%",
    .description = "Sets timesharing CPU job execution scheduling priority (1..100).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_priority_register(void) {
    lang_desc_register(&g_priority_desc);
}

BppError stmt_priority_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PRIORITY) {
        lex_next(lex);
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_NUMBER && val.type != VAL_INTEGER) {
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
