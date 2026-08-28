// FILENAME: extend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (extend.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the EXTEND statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/extend.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_extend_register(void) {
    static const MicroLibMetadata meta_extend = {
        .name = "EXTEND",
        .category = "Compiler Directives",
        .syntax = "EXTEND",
        .help_text = "Enables DEC BASIC-PLUS extended syntax (multi-character identifiers and continuations).",
        .error_codes = "None"
    };
    microlib_register(&meta_extend);

    static const MicroLibMetadata meta_noextend = {
        .name = "NOEXTEND",
        .category = "Compiler Directives",
        .syntax = "NO EXTEND | NOEXTEND",
        .help_text = "Disables extended syntax restrictions (classic Minimal BASIC variable rules).",
        .error_codes = "None"
    };
    microlib_register(&meta_noextend);
}

BppError stmt_extend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_noextend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}
