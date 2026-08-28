// FILENAME: complex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (complex.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the COMPLEX statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/complex.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_complex_register(void) {
    static const MicroLibMetadata meta = {
        .name = "COMPLEX",
        .category = "Variables & Declarations",
        .syntax = "COMPLEX var1 [, var2, arr(dim1 [, dim2])]",
        .help_text = "Declares complex variables and arrays with real and imaginary components (Dartmouth DTSS).",
        .error_codes = "Error 2: Syntax error, Error 9: Subscript out of range"
    };
    microlib_register(&meta);
}

BppError stmt_complex_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_COMPLEX) {
        lex_next(lex);
    }

    VariableContext *vc = vm_get_var(vm);
    ArrayContext *arr_ctx = vm_get_arr(vm);

    while (true) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected variable name in COMPLEX statement";
            return err;
        }

        char var_name[64];
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        memcpy(var_name, var_tok.start, len);
        var_name[len] = '\0';

        BppToken peek = lex_peek(lex);
        if (peek.type == TOK_LPAREN) {
            lex_next(lex); // Consume '('
            int dims[8];
            int dim_count = 0;
            while (dim_count < 8) {
                BValue bval = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                dims[dim_count++] = (int)bval.as.number;
                BppToken sep = lex_peek(lex);
                if (sep.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (sep.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')' in COMPLEX array bounds";
                    return err;
                }
            }
            BppError dim_err = arr_dim(arr_ctx, var_name, dim_count, dims);
            if (dim_err.code != 0) {
                return dim_err;
            }
            arr_set_type(arr_ctx, var_name, VAL_COMPLEX);
        } else {
            BValue zval = { .type = VAL_COMPLEX };
            zval.as.complex_val.real = 0.0;
            zval.as.complex_val.imag = 0.0;
            var_assign(vc, var_name, zval);
        }

        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            continue;
        }
        break;
    }

    return err;
}
