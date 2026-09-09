// FILENAME: form.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (form.h, string.c)
// Provides runtime implementation for the FORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/form.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_form_desc = {
    .name = "FORM",
    .category = "Input / Output",
    .syntax = "FORM format_specifier_list",
    .description = "Declares a structured data format specification (SDS 940 / DEC PDP-10 Super BASIC).",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_form_register(void) {
    lang_desc_register(&g_form_desc);
}

BppError stmt_form_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm;

    // Consume all remaining tokens on this line till TOK_EOF
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOF) {
            break;
        }
        lex_next(lex);
    }
    return err;
}
