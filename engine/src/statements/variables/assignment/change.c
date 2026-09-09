// FILENAME: change.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
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
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_change_desc = {
    .name = "CHANGE",
    .category = "Variables & Memory",
    .syntax = "CHANGE string_expr TO num_array | CHANGE num_array TO string_var",
    .description = "Converts between string and a 1D numeric array of ASCII codes (SDS 940 / DEC PDP-10 Super BASIC).",
    .error_summary = "Error 2: Syntax error, Error 9: Subscript out of range, Error 13: Type mismatch, Error 5: Illegal function call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_change_register(void) {
    lang_desc_register(&g_change_desc);
}

BppError stmt_change_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken first_tok = lex_peek(lex);
    if (first_tok.type == TOK_KEYWORD && (first_tok.as.keyword == KW_CHANGE ||
                                          first_tok.as.keyword == KW_UNPACK ||
                                          first_tok.as.keyword == KW_PACK)) {
        lex_next(lex);
        first_tok = lex_peek(lex);
    } else if (first_tok.type == TOK_IDENT &&
               ((first_tok.length == 6 && runtime_strncasecmp(first_tok.start, "CHANGE", 6) == 0) ||
                (first_tok.length == 6 && runtime_strncasecmp(first_tok.start, "UNPACK", 6) == 0) ||
                (first_tok.length == 4 && runtime_strncasecmp(first_tok.start, "PACK", 4) == 0))) {
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
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && runtime_strncasecmp(to_tok.start, "TO", 2) == 0);
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
        runtime_memcpy(arr_name, arr_tok.start, arr_len);
        arr_name[arr_len] = '\0';

        const char *data = str_data(sval.as.string);
        int slen = (int)str_len(sval.as.string);

        ArrayContext *arr_ctx = vm_get_arr(vm);
        int base = arr_get_option_base(arr_ctx);
        int idx = (base == 1) ? 1 : 0;
        int char_start = idx + 1;

        if (!arr_exists(arr_ctx, arr_name)) {
            int needed = (base == 1) ? (slen + 1) : slen;
            int bound = (needed > 10) ? needed : 10;
            arr_dim(arr_ctx, arr_name, 1, &bound);
        }

        // Set A(0) (or A(1) if base 1) = length
        BValue *elem0 = arr_get_element(arr_ctx, arr_name, 1, &idx, &err);
        if (elem0) {
            elem0->type = VAL_NUMBER;
            elem0->as.number = (double)slen;
        }

        // Set ASCII char codes starting at char_start
        for (int i = 0; i < slen; i++) {
            int idx_i = char_start + i;
            BValue *elemi = arr_get_element(arr_ctx, arr_name, 1, &idx_i, &err);
            if (elemi) {
                elemi->type = VAL_NUMBER;
                elemi->as.number = (double)((unsigned char)data[i]);
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
        runtime_memcpy(arr_name, arr_tok.start, arr_len);
        arr_name[arr_len] = '\0';

        // Check for TO keyword
        BppToken to_tok = lex_next(lex);
        bool is_to = (to_tok.type == TOK_KEYWORD && to_tok.as.keyword == KW_TO) ||
                     (to_tok.type == TOK_IDENT && to_tok.length == 2 && runtime_strncasecmp(to_tok.start, "TO", 2) == 0);
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
        runtime_memcpy(str_var, str_tok.start, str_var_len);
        str_var[str_var_len] = '\0';

        ArrayContext *arr_ctx = vm_get_arr(vm);
        int base = arr_get_option_base(arr_ctx);
        int idx = (base == 1) ? 1 : 0;
        int char_start = idx + 1;

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

        char stack_buf[512];
        char *buf = stack_buf;
        if ((size_t)slen + 1 > sizeof(stack_buf)) {
            buf = (char *)mem_scratch_alloc(vm_get_mem(vm), (size_t)slen + 1);
            if (!buf) {
                err.code = ERR_OUT_OF_MEMORY;
                err.message = "Out of memory in CHANGE";
                return err;
            }
        }

        for (int i = 0; i < slen; i++) {
            int idx_i = char_start + i;
            BValue *elemi = arr_get_element(arr_ctx, arr_name, 1, &idx_i, &err);
            if (elemi) {
                buf[i] = (char)((int)elemi->as.number & 0xFF);
            } else {
                buf[i] = '\0';
            }
        }
        buf[slen] = '\0';

        BValue str_val;
        runtime_memset(&str_val, 0, sizeof(str_val));
        str_val.type = VAL_STRING;
        str_val.as.string = str_create(vm_get_str(vm), buf, (size_t)slen);

        var_assign(vm_get_var(vm), str_var, str_val);
        str_release(vm_get_str(vm), str_val.as.string);

        return err;
    }
}
