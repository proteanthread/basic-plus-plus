// FILENAME: stmt_shuffle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, language_descriptor.h, strings.h, variables.h)
// NEEDS: libengine (eval.h, lexer.h, rnd.h, stmt_shuffle.h, vm.h)
// Provides runtime implementation for the SHUFFLE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/stmt_shuffle.h"
#include "eval/functions/math/random/rnd.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "runtime/arrays.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "vm/vm.h"

static const LangDesc g_shuffle_desc = {
    .name = "SHUFFLE",
    .category = "Variables & Memory",
    .syntax = "SHUFFLE arr[()] [, seed] | SHUFFLE (arr [, seed]) | SHUFFLE str$ [, seed]",
    .description = "Randomly shuffles array elements or string characters in-place using Fisher-Yates algorithm.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_shuffle_register(void) {
    lang_desc_register(&g_shuffle_desc);
}

BppError stmt_shuffle_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in SHUFFLE";
        return err;
    }

    bool outer_paren = false;
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex);
        outer_paren = true;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char target_name[256];
        size_t clen = (tok.length < sizeof(target_name) - 1) ? tok.length : sizeof(target_name) - 1;
        runtime_memcpy(target_name, tok.start, clen);
        target_name[clen] = '\0';
        lex_next(lex); // Consume variable/array identifier

        // Check for optional array empty parentheses: arr()
        BppToken next = lex_peek(lex);
        if (next.type == TOK_LPAREN) {
            lex_next(lex);
            next = lex_peek(lex);
            if (next.type == TOK_RPAREN) {
                lex_next(lex);
            } else {
                err.code = 2;
                err.message = "Expected ')' in SHUFFLE array syntax";
                return err;
            }
        }

        // Check for optional comma and seed expression
        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // Consume comma
            BValue seed_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (seed_val.type == VAL_INTEGER || seed_val.type == VAL_NUMBER) {
                double s = seed_val.as.number;
                if (s >= 0.0) s = -s;
                if (s == 0.0) s = -12345.0;
                BValue seed_arg = { .type = VAL_NUMBER, .as.number = s };
                func_rnd_eval(vm, "RND", 1, &seed_arg, &err);
            }
        }

        if (outer_paren) {
            if (lex_peek(lex).type == TOK_RPAREN) {
                lex_next(lex);
            } else {
                err.code = 2;
                err.message = "Expected ')' to close SHUFFLE statement";
                return err;
            }
        }

        // Case 1: Target is an Array
        if (arr_exists(vm_get_arr(vm), target_name)) {
            int total_size = 0;
            BValue *elements = arr_get_flat_elements(vm_get_arr(vm), target_name, &total_size);
            if (elements && total_size > 1) {
                for (int i = total_size - 1; i > 0; i--) {
                    double r = func_rnd_eval(vm, "RND", 0, NULL, &err).as.number;
                    int j = (int)(r * (double)(i + 1));
                    if (j < 0) j = 0;
                    if (j > i) j = i;
                    BValue temp = elements[i];
                    elements[i] = elements[j];
                    elements[j] = temp;
                }
            }
            return err;
        }

        // Case 2: Target is a String Variable
        BValue *bval = var_lookup(vm_get_var(vm), target_name, false);
        if (bval) {
            if (bval->type == VAL_STRING && bval->as.string) {
                const char *s_data = str_data(bval->as.string);
                size_t s_len = str_len(bval->as.string);
                if (s_len > 1 && s_data) {
                    char *buf = (char *)runtime_malloc(s_len + 1);
                    if (!buf) {
                        err.code = 7;
                        err.message = "Out of memory in SHUFFLE";
                        return err;
                    }
                    runtime_memcpy(buf, s_data, s_len);
                    buf[s_len] = '\0';

                    for (size_t i = s_len; i > 1; i--) {
                        double r = func_rnd_eval(vm, "RND", 0, NULL, &err).as.number;
                        size_t j = (size_t)(r * (double)i);
                        if (j >= i) j = i - 1;
                        char tmp = buf[i - 1];
                        buf[i - 1] = buf[j];
                        buf[j] = tmp;
                    }

                    BppStringRef new_str = str_create(vm_get_str(vm), buf, s_len);
                    runtime_free(buf);
                    str_release(vm_get_str(vm), bval->as.string);
                    bval->as.string = new_str;
                }
                return err;
            } else {
                err.code = 13;
                err.message = "Type mismatch: SHUFFLE requires an array or string variable";
                return err;
            }
        }

        err.code = 5;
        err.message = "Illegal function call: Array or variable not found in SHUFFLE";
        return err;
    }

    err.code = 2;
    err.message = "Syntax error: Expected array or variable identifier after SHUFFLE";
    return err;
}
