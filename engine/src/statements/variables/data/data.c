// FILENAME: data.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (data.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DATA statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/data.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_data_desc = {
    .name = "DATA",
    .category = "Variables & Memory",
    .syntax = "DATA constant1 [, constant2...]",
    .description = "Stores static numeric and string constants to be read sequentially into variables.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_data_register(void) {
    lang_desc_register(&g_data_desc);
}

BppError stmt_data_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }
    return err;
}
