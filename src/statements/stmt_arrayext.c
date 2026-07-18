/**
 * @file stmt_arrayext.c
 * @brief ARRAY MAP, ARRAY FILTER, ARRAY REDUCE statements.
 */

#include "bpp_stmt.h"
#include "bpp_lexer.h"
#include "bpp_arrays.h"
#include "bpp_eval.h"
#include "bpp_vm.h"
#include "bpp_strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern BppError arrayext_execute_map(VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str);

BppError stmt_arrayext_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    BppKeywordId op = tok.as.keyword;
    if (op != KW_MAP && op != KW_FILTER && op != KW_REDUCE) {
        err.code = 2; err.message = "Expected MAP, FILTER, or REDUCE after ARRAY";
        return err;
    }
    lex_next(lex); /* Consume MAP/FILTER/REDUCE */

    /* Get source array */
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected source array name";
        return err;
    }
    char src_arr[256];
    size_t slen = (tok.length < sizeof(src_arr) - 1) ? tok.length : sizeof(src_arr) - 1;
    memcpy(src_arr, tok.start, slen);
    src_arr[slen] = '\0';

    if (lex_next(lex).type != TOK_LPAREN || lex_next(lex).type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected () after source array name";
        return err;
    }

    char dst_arr[256];
    snprintf(dst_arr, sizeof(dst_arr), "%s", src_arr); /* Default in-place */

    tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TO) { /* Wait, TO is not a keyword? Let's check lexer. Actually, TO is TOK_IDENT usually? No, it's KW_TO. Wait, there's KW_TO? In eval.c it's usually KW_TO. Let's just use TOK_IDENT and check "TO" */
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected destination array name after TO";
            return err;
        }
        size_t dlen = (tok.length < sizeof(dst_arr) - 1) ? tok.length : sizeof(dst_arr) - 1;
        memcpy(dst_arr, tok.start, dlen);
        dst_arr[dlen] = '\0';
        if (lex_next(lex).type != TOK_LPAREN || lex_next(lex).type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected () after destination array name";
            return err;
        }
        tok = lex_peek(lex);
    }

    char fn_name[256] = "";
    char label_name[256] = "";
    char expr_str[256] = "";

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex);
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected function name after USING";
            return err;
        }
        size_t flen = (tok.length < sizeof(fn_name) - 1) ? tok.length : sizeof(fn_name) - 1;
        memcpy(fn_name, tok.start, flen);
        fn_name[flen] = '\0';
    } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOSUB) {
        lex_next(lex);
        tok = lex_next(lex);
        /* Expect label */
        if (tok.type != TOK_IDENT && tok.type != TOK_NUMBER) {
            err.code = 2; err.message = "Expected label after GOSUB";
            return err;
        }
        size_t llen = (tok.length < sizeof(label_name) - 1) ? tok.length : sizeof(label_name) - 1;
        memcpy(label_name, tok.start, llen);
        label_name[llen] = '\0';
    } else if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue expr_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (expr_val.type != VAL_STRING || !expr_val.as.string) {
            err.code = 13; err.message = "Expected string expression for ARRAY evaluation";
            return err;
        }
        strncpy(expr_str, str_data(expr_val.as.string), sizeof(expr_str) - 1);
        str_release(vm_get_str(vm), expr_val.as.string);
    } else {
        err.code = 2; err.message = "Expected USING, GOSUB, or string expression";
        return err;
    }

    if (op == KW_MAP) {
        return arrayext_execute_map(vm, src_arr, dst_arr, fn_name, label_name, expr_str);
    } else if (op == KW_FILTER) {
        /* Not yet implemented in this file directly, will call module */
    } else if (op == KW_REDUCE) {
        /* Not yet implemented */
    }

    return err;
}
