// FILENAME: erase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (erase.h, string.c)
// Provides runtime implementation for the ERASE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/erase.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_erase_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ERASE",
        .category = "Variables & Memory",
        .syntax = "ERASE array_name1 [, array_name2...]",
        .help_text = "Eliminates dynamic arrays from memory and reallocates storage space.",
        .error_codes = "Error 2: Syntax Error, Error 10: Array Not Dimensioned"
    };
    microlib_register(&meta);
}

BppError stmt_erase_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *ac = vm_get_arr(vm);

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Syntax Error in ERASE (expected array name)";
            return err;
        }

        char arr_name[64];
        if (tok.length >= sizeof(arr_name)) tok.length = sizeof(arr_name) - 1;
        memcpy(arr_name, tok.start, tok.length);
        arr_name[tok.length] = '\0';

        arr_erase(ac, arr_name);

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}
