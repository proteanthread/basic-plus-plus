// FILENAME: def_usr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (def_usr.h, string.c)
// Provides runtime implementation for the DEF_USR statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/def_usr.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_def_usr_register(void) {
    MicroLibMetadata meta = {
        .name = "DEF USR",
        .category = "System & Memory",
        .syntax = "DEF USR[digit%] = address%",
        .help_text = "Specifies the starting address of an assembly language subroutine called by USR.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_def_usr_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    memset(&err, 0, sizeof(err));
    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        lex_next(lex);
    }
    return err;
}
