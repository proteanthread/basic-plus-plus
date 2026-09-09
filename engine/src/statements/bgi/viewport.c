// FILENAME: viewport.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, viewport.h)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the VIEWPORT statement in BASIC++.
//
// ---- Includes ----

#include "statements/bgi/viewport.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_viewport_desc = {
    .name = "VIEWPORT",
    .category = "Graphics & Sound",
    .syntax = "VIEWPORT xmin, xmax, ymin, ymax",
    .description = "ECMA-116 standard statement to specify a normalized graphics viewport boundary.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_viewport_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    bool has_bracket = false;
    if (lex_peek(lex).type == TOK_LBRACKET) {
        lex_next(lex);
        has_bracket = true;
    }

    BValue xmin = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else if (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex);
        if (lex_peek(lex).type == TOK_PERIOD) lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected comma in VIEWPORT"; return err;
    }

    BValue xmax = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected comma in VIEWPORT"; return err;
    }

    BValue ymin = eval_expression(vm, lex, &err); if (err.code != 0) return err;
    if (lex_peek(lex).type == TOK_COMMA) {
        lex_next(lex);
    } else if (lex_peek(lex).type == TOK_PERIOD) {
        lex_next(lex);
        if (lex_peek(lex).type == TOK_PERIOD) lex_next(lex);
    } else {
        err.code = 2; err.message = "Expected comma in VIEWPORT"; return err;
    }

    BValue ymax = eval_expression(vm, lex, &err); if (err.code != 0) return err;

    if (has_bracket) {
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
    }

    (void)xmin; (void)xmax; (void)ymin; (void)ymax;
    return err;
}

void stmt_viewport_register(void) {
    lang_desc_register(&g_viewport_desc);
}
