// FILENAME: arrayfill.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (arrayfill.h, eval.h, eval.c, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the ARRAYFILL statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/arrayfill.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_arrayfill_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ARRAYFILL",
        .category = "Arrays & Matrices",
        .syntax = "ARRAYFILL array_name(), fill_value",
        .help_text = "Fills all elements of the specified array with the given value.",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_arrayfill_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    ArrayContext *ac = vm_get_arr(vm);
    StringContext *sc = vm_get_str(vm);

    BppToken name_tok = lex_next(lex);
    if (name_tok.type != TOK_IDENT && name_tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected array name in ARRAYFILL";
        return err;
    }

    char arr_name[64];
    size_t len = (name_tok.length < sizeof(arr_name) - 1) ? name_tok.length : sizeof(arr_name) - 1;
    memcpy(arr_name, name_tok.start, len);
    arr_name[len] = '\0';

    // Optional () after array name
    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BppToken close_tok = lex_next(lex);
        if (close_tok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' after array name in ARRAYFILL";
            return err;
        }
    }

    BppToken comma_tok = lex_next(lex);
    if (comma_tok.type != TOK_COMMA) {
        err.code = 2; err.message = "Expected ',' before fill value in ARRAYFILL";
        return err;
    }

    BValue fill_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int total_size = 0;
    BValue *elements = arr_get_flat_elements(ac, arr_name, &total_size);
    if (!elements || total_size <= 0) {
        if (fill_val.type == VAL_STRING && fill_val.as.string) str_release(sc, fill_val.as.string);
        err.code = 9; err.message = "Array not dimensioned or empty";
        return err;
    }

    for (int i = 0; i < total_size; i++) {
        if (elements[i].type == VAL_STRING && elements[i].as.string) {
            str_release(sc, elements[i].as.string);
            elements[i].as.string = NULL;
        }

        if (fill_val.type == VAL_STRING) {
            elements[i].type = VAL_STRING;
            if (fill_val.as.string) {
                elements[i].as.string = str_create(sc, str_data(fill_val.as.string), str_len(fill_val.as.string));
            } else {
                elements[i].as.string = str_create(sc, "", 0);
            }
        } else {
            elements[i].type = fill_val.type;
            elements[i].as.number = fill_val.as.number;
        }
    }

    if (fill_val.type == VAL_STRING && fill_val.as.string) {
        str_release(sc, fill_val.as.string);
    }

    return err;
}
