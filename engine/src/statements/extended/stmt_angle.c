// FILENAME: stmt_angle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libkernel (vm.h, lexer.h, errors.h)
// Provides runtime implementation for DEGREE, RADIAN, and GRAD angle mode statements.
//
// ---- Includes ----

#include "statements/extended/stmt_angle.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"

static const LangDesc g_degree_desc = {
    .name = "DEGREE",
    .category = "Math & Trigonometry",
    .syntax = "DEGREE",
    .description = "Sets trigonometric evaluation mode to degrees (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_radian_desc = {
    .name = "RADIAN",
    .category = "Math & Trigonometry",
    .syntax = "RADIAN",
    .description = "Sets trigonometric evaluation mode to radians (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_grad_desc = {
    .name = "GRAD",
    .category = "Math & Trigonometry",
    .syntax = "GRAD",
    .description = "Sets trigonometric evaluation mode to gradians (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_angle_register(void) {
    lang_desc_register(&g_degree_desc);
    lang_desc_register(&g_radian_desc);
    lang_desc_register(&g_grad_desc);
}

BppError stmt_degree_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DEGREE) {
        lex_next(lex);
    }
    vm_set_angle_mode(vm, 1); // 1 = DEGREE
    return err;
}

BppError stmt_radian_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_RADIAN) {
        lex_next(lex);
    }
    vm_set_angle_mode(vm, 0); // 0 = RADIAN
    return err;
}

BppError stmt_grad_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GRAD) {
        lex_next(lex);
    }
    vm_set_angle_mode(vm, 2); // 2 = GRAD
    return err;
}
