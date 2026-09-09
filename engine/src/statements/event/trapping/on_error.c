// FILENAME: on_error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_error.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the ON_ERROR statement in BASIC++.
//
// ---- Includes ----

#include "statements/event/trapping/on_error.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_on_error_desc = {
    .name = "ON ERROR",
    .category = "Event Trapping",
    .syntax = "ON ERROR GOTO {line_label | 0}",
    .description = "Enables error-trapping routine and specifies the first statement of the handler.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_on_error_register(void) {
    lang_desc_register(&g_on_error_desc);
}

BppError stmt_on_error_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) {
        lex_next(lex);
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            double target_line = val.as.number;
            vm_set_error_trap(vm, target_line);
        }
    } else {
        err.code = 2;
        err.message = "Syntax error in ON ERROR (expected GOTO)";
    }

    return err;
}

BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex) {
    return stmt_on_error_handler(vm, lex);
}

BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_NUMBER) {
        int code = (int)val.as.number;
        err.code = code;
        err.message = "User Error";
        err.line = vm_get_current_line(vm);
        vm_set_error(vm, code, "User Error");
    }

    return err;
}
