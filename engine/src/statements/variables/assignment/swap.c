// FILENAME: swap.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exchange.c)
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, string.c, swap.h)
// Provides runtime implementation for the SWAP statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/swap.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_swap_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SWAP",
        .category = "Variables & Memory",
        .syntax = "SWAP variable1, variable2",
        .help_text = "Exchanges the values of two variables or array elements of identical types.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_swap_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok1 = lex_next(lex);
    if (tok1.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected variable)";
        return err;
    }
    char var1[64];
    if (tok1.length >= sizeof(var1)) tok1.length = sizeof(var1) - 1;
    memcpy(var1, tok1.start, tok1.length);
    var1[tok1.length] = '\0';

    bool is_arr1 = false;
    int indices1[8];
    int dims1 = 0;
    BppToken peek1 = lex_peek(lex);
    if (peek1.type == TOK_LPAREN) {
        is_arr1 = true;
        lex_next(lex); // Consume '('
        while (dims1 < 8) {
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                err.code = 13; err.message = "Type Mismatch in array subscript (expected numeric index)";
                return err;
            }
            indices1[dims1++] = (int)idx_val.as.number;
            peek1 = lex_peek(lex);
            if (peek1.type == TOK_COMMA) {
                lex_next(lex);
            } else if (peek1.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err.code = 2; err.message = "Syntax error in array subscript (expected ',' or ')')";
                return err;
            }
        }
    }

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected comma)";
        return err;
    }

    BppToken tok2 = lex_next(lex);
    if (tok2.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax Error in SWAP (expected second variable)";
        return err;
    }
    char var2[64];
    if (tok2.length >= sizeof(var2)) tok2.length = sizeof(var2) - 1;
    memcpy(var2, tok2.start, tok2.length);
    var2[tok2.length] = '\0';

    bool is_arr2 = false;
    int indices2[8];
    int dims2 = 0;
    BppToken peek2 = lex_peek(lex);
    if (peek2.type == TOK_LPAREN) {
        is_arr2 = true;
        lex_next(lex); // Consume '('
        while (dims2 < 8) {
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                err.code = 13; err.message = "Type Mismatch in array subscript (expected numeric index)";
                return err;
            }
            indices2[dims2++] = (int)idx_val.as.number;
            peek2 = lex_peek(lex);
            if (peek2.type == TOK_COMMA) {
                lex_next(lex);
            } else if (peek2.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err.code = 2; err.message = "Syntax error in array subscript (expected ',' or ')')";
                return err;
            }
        }
    }

    VariableContext *vc = vm_get_var(vm);
    ArrayContext *arr_ctx = vm_get_arr(vm);
    bool str1 = (var1[strlen(var1) - 1] == '$');
    bool str2 = (var2[strlen(var2) - 1] == '$');

    if (str1 != str2) {
        err.code = 13;
        err.message = "Type Mismatch in SWAP";
        return err;
    }

    BValue *v1_ptr = NULL;
    if (is_arr1) {
        v1_ptr = arr_get_element(arr_ctx, var1, dims1, indices1, &err);
        if (err.code != 0 || !v1_ptr) return err;
    } else {
        v1_ptr = var_lookup(vc, var1, true);
    }

    BValue *v2_ptr = NULL;
    if (is_arr2) {
        v2_ptr = arr_get_element(arr_ctx, var2, dims2, indices2, &err);
        if (err.code != 0 || !v2_ptr) return err;
    } else {
        v2_ptr = var_lookup(vc, var2, true);
    }

    if (v1_ptr && v2_ptr) {
        BValue tmp = *v1_ptr;
        *v1_ptr = *v2_ptr;
        *v2_ptr = tmp;
    }

    return err;
}
