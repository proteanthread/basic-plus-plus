// FILENAME: image.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (image.h, string.c)
// Provides runtime implementation for the IMAGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/image.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_image_desc = {
    .name = "IMAGE",
    .category = "Input / Output",
    .syntax = "IMAGE: format_template_specifiers",
    .description = "Declares a line format template referenced by PRINT USING or PRINT IN FORM (SDS 940 / DEC PDP-10 Super BASIC).",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_image_register(void) {
    lang_desc_register(&g_image_desc);
}

BppError stmt_image_handler(VMContext *vm, LexerContext *lex) {
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
