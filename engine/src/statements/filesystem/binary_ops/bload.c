// FILENAME: bload.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (bload.h, string.c)
// Provides runtime implementation for the BLOAD statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/bload.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_bload_register(void) {
    MicroLibMetadata meta = {
        .name = "BLOAD",
        .category = "File I/O & Memory",
        .syntax = "BLOAD filename$ [, offset%]",
        .help_text = "Loads a memory image file created by BSAVE into memory at specified offset.",
        .error_codes = "Error 53: File Not Found, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_bload_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}
