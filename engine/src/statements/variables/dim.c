/**
 * @file dim.c
 * @brief DIM, REDIM, and ERASE statement handlers for BASIC++.
 */
#include "statements/variables/dim.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_dim_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ERASE",
        .category = "Variables & Memory",
        .syntax = "ERASE array_name1 [, array_name2...]",
        .help_text = "Eliminates dynamic arrays from memory and reallocates storage space.",
        .error_codes = "Error 2: Syntax Error, Error 10: Array Not Dimensioned"
    };
    microlib_register(&meta);
}

BppError stmt_dim_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    return err;
}

BppError stmt_redim_handler(VMContext *vm, LexerContext *lex) {
    return stmt_dim_handler(vm, lex);
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
