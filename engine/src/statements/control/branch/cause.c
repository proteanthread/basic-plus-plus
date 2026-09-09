// FILENAME: cause.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (cause.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CAUSE statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/branch/cause.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_cause_desc = {
    .name = "CAUSE",
    .category = "Control Flow",
    .syntax = "CAUSE ERROR error_code",
    .description = "ECMA-116 standard statement to raise a runtime error.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_cause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ERROR) ||
        (tok.type == TOK_IDENT && tok.length == 5 && runtime_strncasecmp(tok.start, "ERROR", 5) == 0)) {
        lex_next(lex);
    }

    BValue code_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (code_val.type != VAL_NUMBER) {
        err.code = 2; err.message = "Expected numeric error code after CAUSE ERROR";
        return err;
    }

    int code = (int)code_val.as.number;
    err.code = code;
    err.message = "User Caused Error";
    return err;
}

void stmt_cause_register(void) {
    lang_desc_register(&g_cause_desc);
}
