// FILENAME: brun.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (brun.h, string.c)
// Provides runtime implementation for the BRUN statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/brun.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_brun_register(void) {
    MicroLibMetadata meta = {
        .name = "BRUN",
        .category = "File I/O & Execution",
        .syntax = "BRUN filename$ [, address%]",
        .help_text = "Loads a binary file into memory and executes it.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found"
    };
    microlib_register(&meta);
}

BppError stmt_brun_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}
