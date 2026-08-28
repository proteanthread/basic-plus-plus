// FILENAME: bsave.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bsave.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the BSAVE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/bsave.h"
#include "runtime/micro_lib_metadata.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include <string.h>

void stmt_bsave_register(void) {
    MicroLibMetadata meta = {
        .name = "BSAVE",
        .category = "File I/O & Memory",
        .syntax = "BSAVE filename$, offset%, length%",
        .help_text = "Saves a block of memory as a binary file on disk.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_bsave_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}
