// FILENAME: change.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (change.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the CHANGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/change.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_change_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CHANGE",
        .category = "Variables & Memory",
        .syntax = "CHANGE string_expr TO num_array | CHANGE num_array TO string_var",
        .help_text = "Converts between string and a 1D numeric array of ASCII codes (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax error, Error 9: Subscript out of range, Error 13: Type mismatch, Error 5: Illegal function call"
    };
    microlib_register(&meta);
}

BppError stmt_change_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken first_tok = lex_peek(lex);
    if (first_tok.type == TOK_KEYWORD && first_tok.as.keyword == KW_CHANGE) {
        lex_next(lex);
        first_tok = lex_peek(lex);
    }

    if (first_tok.type == TOK_EOF || first_tok.type == TOK_EOL) {
        err.code = ERR_SYNTAX;
        err.message = "Expected arguments for CHANGE statement";
        return err;
    }

    // Check if first argument is a string expression or an array name
    bool is_string_source = false;
    if (first_tok.type == TOK_STRING) {
        is_string_source = true;
    } else if (first_tok.type == TOK_IDENT) {
        // Check if ends with '$' or is a declared string/function
        if (first_tok.length > 0 && first_tok.start[first_tok.length - 1] == '$') {
            is_string_source = true;
        }
    }

    if (is_string_source) {
        // 1. CHANGE string_expr TO num_array
        BValue sval = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (sval.type != VAL_STRING || !sval.as.string) {
            if (sval.type == VAL_STRING && sval.as.string) {
                str_release(vm_get_str(vm), sval.as.string);
            }
            err.code = ERR_TYPE_MISMATCH;
            err.message = "Type mismatch: expected string expression in CHANGE";
            return err;
        }

        // Check for TO keyword
        BppToken to_tok = lex_next(lex);
        bool is_to = (to_tok.type == TOK_KEYWORD && to_tok.as.keyword == KW_TO) ||
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && strncasecmp(to_tok.start, "TO", 2) == 0);
        if (!is_to) {
            str_release(vm_get_str(vm), sval.as.string);
            err.code = ERR_SYNTAX;
            err.message = "Expected 'TO' in CHANGE statement";
            return err;
        }

        // Target array name
        BppToken arr_tok = lex_next(lex);
        if (arr_tok.type != TOK_IDENT && arr_tok.type != TOK_KEYWORD) {
            str_release(vm_get_str(vm), sval.as.string);
            err.code = ERR_SYNTAX;
            err.message = "Expected target numeric array name in CHANGE";
            return err;
        }

        char arr_name[64];
        size_t arr_len = (arr_tok.length < sizeof(arr_name) - 1) ? arr_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, arr_tok.start, arr_len);
        arr_name[arr_len] = '\0';

        const char *data = str_data(sval.as.string);
        int slen = (int)str_len(sval.as.string);

        ArrayContext *arr_ctx = vm_get_arr(vm);
        if (!arr_exists(arr_ctx, arr_name)) {
            int bound = (slen > 10) ? slen : 10;
            arr_dim(arr_ctx, arr_name, 1, &bound);
        }

        // Set A(0) = length
        int idx = 0;
        BValue *elem0 = arr_get_element(arr_ctx, arr_name, 1, &idx, &err);
        if (elem0) {
            elem0->type = VAL_NUMBER;
            elem0->as.number = (double)slen;
        }

        // Set A(1)..A(slen) = ASCII char codes
        for (int i = 1; i <= slen; i++) {
            int idx_i = i;
            BValue *elemi = arr_get_element(arr_ctx, arr_name, 1, &idx_i, &err);
            if (elemi) {
                elemi->type = VAL_NUMBER;
                elemi->as.number = (double)((unsigned char)data[i - 1]);
            }
        }

        str_release(vm_get_str(vm), sval.as.string);
        return err;
    } else {
        // 2. CHANGE num_array TO string_var
        BppToken arr_tok = lex_next(lex);
        if (arr_tok.type != TOK_IDENT && arr_tok.type != TOK_KEYWORD) {
            err.code = ERR_SYNTAX;
            err.message = "Expected array name or string in CHANGE";
            return err;
        }

        char arr_name[64];
        size_t arr_len = (arr_tok.length < sizeof(arr_name) - 1) ? arr_tok.length : sizeof(arr_name) - 1;
        memcpy(arr_name, arr_tok.start, arr_len);
        arr_name[arr_len] = '\0';

        // Check for TO keyword
        BppToken to_tok = lex_next(lex);
        bool is_to = (to_tok.type == TOK_KEYWORD && to_tok.as.keyword == KW_TO) ||
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && strncasecmp(to_tok.start, "TO", 2) == 0);
        if (!is_to) {
            err.code = ERR_SYNTAX;
            err.message = "Expected 'TO' in CHANGE statement";
            return err;
        }

        // Target string variable
        BppToken str_tok = lex_next(lex);
        if (str_tok.type != TOK_IDENT && str_tok.type != TOK_KEYWORD) {
            err.code = ERR_SYNTAX;
            err.message = "Expected target string variable name in CHANGE";
            return err;
        }

        char str_var[64];
        size_t str_var_len = (str_tok.length < sizeof(str_var) - 1) ? str_tok.length : sizeof(str_var) - 1;
        memcpy(str_var, str_tok.start, str_var_len);
        str_var[str_var_len] = '\0';

        ArrayContext *arr_ctx = vm_get_arr(vm);
        int idx = 0;
        BValue *elem0 = arr_get_element(arr_ctx, arr_name, 1, &idx, &err);
        if (!elem0 || err.code != 0) {
            if (err.code == 0) err.code = ERR_SUBSCRIPT_OUT_OF_RANGE;
            return err;
        }

        int slen = (int)elem0->as.number;
        if (slen < 0 || slen > 65535) {
            err.code = ERR_ILLEGAL_FUNCTION_CALL;
            err.message = "Illegal length in CHANGE array to string";
            return err;
        }

        char *buf = (char *)malloc((size_t)slen + 1);
        if (!buf) {
            err.code = ERR_OUT_OF_MEMORY;
            err.message = "Out of memory in CHANGE";
            return err;
        }

        for (int i = 1; i <= slen; i++) {
            int idx_i = i;
            BValue *elemi = arr_get_element(arr_ctx, arr_name, 1, &idx_i, &err);
            if (elemi) {
                buf[i - 1] = (char)((int)elemi->as.number & 0xFF);
            } else {
                buf[i - 1] = '\0';
            }
        }
        buf[slen] = '\0';

        BValue str_val;
        memset(&str_val, 0, sizeof(str_val));
        str_val.type = VAL_STRING;
        str_val.as.string = str_create(vm_get_str(vm), buf, (size_t)slen);

        var_assign(vm_get_var(vm), str_var, str_val);
        str_release(vm_get_str(vm), str_val.as.string);
        free(buf);

        return err;
    }
}
