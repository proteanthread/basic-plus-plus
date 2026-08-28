// FILENAME: form.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (form.h, string.c)
// Provides runtime implementation for the FORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/form.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_form_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FORM",
        .category = "Input / Output",
        .syntax = "FORM format_specifier_list",
        .help_text = "Declares a structured data format specification (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_form_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
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
