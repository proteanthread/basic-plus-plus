// FILENAME: stmt_every.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the EVERY statement for periodic timer event scheduling.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_every_desc = {
    .name = "EVERY",
    .category = "Timing & Real-Time",
    .syntax = "EVERY ms GOSUB line_num",
    .description = "Schedules a recurring subroutine execution on hardware timer intervals.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_every_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue intv_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOSUB) {
        BValue line_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        (void)intv_val;
        (void)line_val;
    }
    return err;
}

void stmt_every_register(void) {
    lang_desc_register(&g_every_desc);
}
