// FILENAME: incr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (arrays.h, language_descriptor.h, strings.h, variables.h)
// NEEDS: libengine (eval.h, incr.h, lexer.h, vm.h)
// Provides runtime implementation for the INCR statement in BASIC++.

#include "statements/variables/assignment/incr.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include <stdbool.h>

static const LangDesc g_incr_desc = {
    .name = "INCR",
    .category = "Variables & Memory",
    .syntax = "INCR variable [, step]",
    .description = "Increments the numeric variable or array element by step (default 1).",
    .error_summary = "Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_incr_register(void) {
    lang_desc_register(&g_incr_desc);
}

BppError stmt_incr_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    VariableContext *vc = vm_get_var(vm);
    ArrayContext *ac = vm_get_arr(vm);

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Syntax error in INCR (expected variable name)";
        return err;
    }

    char var_name[64];
    size_t len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    runtime_memcpy(var_name, tok.start, len);
    var_name[len] = '\0';

    bool is_array = false;
    int num_dims = 0;
    int indices[4] = {0};

    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_LPAREN) {
        is_array = true;
        lex_next(lex); // Consume '('
        while (1) {
            if (num_dims >= 4) {
                err.code = 9; err.message = "Subscript out of range (max 4 dimensions)";
                return err;
            }
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                if (idx_val.type == VAL_STRING && idx_val.as.string) str_release(vm_get_str(vm), idx_val.as.string);
                err.code = 13; err.message = "Type mismatch in array subscript";
                return err;
            }
            indices[num_dims++] = (int)idx_val.as.number;

            BppToken sep = lex_peek(lex);
            if (sep.type == TOK_COMMA) {
                lex_next(lex);
                continue;
            } else if (sep.type == TOK_RPAREN) {
                lex_next(lex);
                break;
            } else {
                err.code = 2; err.message = "Expected ',' or ')' in array subscript";
                return err;
            }
        }
    }

    double step_val = 1.0;
    BppToken comma_tok = lex_peek(lex);
    if (comma_tok.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        BValue s_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (s_val.type == VAL_NUMBER || s_val.type == VAL_INTEGER) {
            step_val = s_val.as.number;
        } else {
            if (s_val.type == VAL_STRING && s_val.as.string) str_release(vm_get_str(vm), s_val.as.string);
            err.code = 13; err.message = "Type mismatch in step value";
            return err;
        }
    }

    if (is_array) {
        BValue *arr_elem = arr_get_element(ac, var_name, num_dims, indices, &err);
        if (!arr_elem) {
            if (err.code == 0) {
                err.code = 9; err.message = "Subscript out of range";
            }
            return err;
        }
        if (arr_elem->type == VAL_STRING) {
            err.code = 13; err.message = "Type mismatch (cannot INCR string)";
            return err;
        }
        arr_elem->as.number += step_val;
    } else {
        BValue *var = var_lookup(vc, var_name, true);
        if (!var) {
            err.code = 4; err.message = "Out of memory";
            return err;
        }
        if (var->type == VAL_STRING) {
            err.code = 13; err.message = "Type mismatch (cannot INCR string)";
            return err;
        }
        if (var->type == VAL_NONE) var->type = VAL_NUMBER;
        var->as.number += step_val;
    }

    return err;
}
