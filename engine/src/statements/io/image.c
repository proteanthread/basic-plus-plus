// FILENAME: image.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (image.h, string.c)
// Provides runtime implementation for the IMAGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/image.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_image_register(void) {
    static const MicroLibMetadata meta = {
        .name = "IMAGE",
        .category = "Input / Output",
        .syntax = "IMAGE: format_template_specifiers",
        .help_text = "Declares a line format template referenced by PRINT USING or PRINT IN FORM (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_image_handler(VMContext *vm, LexerContext *lex) {
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
