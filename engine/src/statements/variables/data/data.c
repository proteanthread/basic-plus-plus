// FILENAME: data.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (data.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DATA statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/data.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_data_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DATA",
        .category = "Variables & Memory",
        .syntax = "DATA constant1 [, constant2...]",
        .help_text = "Stores static numeric and string constants to be read sequentially into variables.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_data_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }
    return err;
}
