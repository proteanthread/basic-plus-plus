// FILENAME: erase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (erase.h, string.c)
// Provides runtime implementation for the ERASE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/erase.h"
#include "runtime/arrays.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_erase_desc = {
    .name = "ERASE",
    .category = "Variables & Memory",
    .syntax = "ERASE array_name1 [, array_name2...]",
    .description = "Eliminates dynamic arrays from memory and reallocates storage space.",
    .error_summary = "Error 2: Syntax Error, Error 10: Array Not Dimensioned",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_erase_register(void) {
    lang_desc_register(&g_erase_desc);
}

BppError stmt_erase_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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
        runtime_memcpy(arr_name, tok.start, tok.length);
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
