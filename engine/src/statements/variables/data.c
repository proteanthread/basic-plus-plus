/**
 * @file data.c
 * @brief DATA, READ, and RESTORE statement handlers for BASIC++.
 */
#include "statements/variables/data.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_data_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DATA",
        .category = "Variables & Memory",
        .syntax = "DATA constant1 [, constant2...] | READ var1 [, var2...] | RESTORE [line_number]",
        .help_text = "Stores static numeric and string constants to be read sequentially into variables.",
        .error_codes = "Error 2: Syntax Error, Error 4: Out of Data, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_data_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_next(lex);
    while (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        tok = lex_next(lex);
    }
    return err;
}

BppError stmt_read_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Syntax error in READ target variable";
            return err;
        }

        char var_name[64];
        if (tok.length >= sizeof(var_name)) tok.length = sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tok.length);
        var_name[tok.length] = '\0';

        int ptr = vm_get_data_ptr(vm);
        int count = vm_get_data_count(vm);
        BppDataPosition *items = vm_get_data_items(vm);

        if (!items || ptr >= count) {
            err.code = 4;
            err.message = "Out of Data";
            return err;
        }

        const char *item_src = items[ptr].pos;
        vm_set_data_ptr(vm, ptr + 1);

        VariableContext *vc = vm_get_var(vm);
        bool is_str = (var_name[strlen(var_name) - 1] == '$');

        if (is_str) {
            char val_buf[256];
            size_t len = 0;
            if (*item_src == '"') {
                item_src++;
                while (*item_src && *item_src != '"' && len < sizeof(val_buf) - 1) {
                    val_buf[len++] = *item_src++;
                }
            } else {
                while (*item_src && *item_src != ',' && *item_src != ':' && *item_src != '\n' && *item_src != '\r' && len < sizeof(val_buf) - 1) {
                    val_buf[len++] = *item_src++;
                }
                while (len > 0 && isspace((unsigned char)val_buf[len - 1])) len--;
            }
            val_buf[len] = '\0';
            StringContext *str_ctx = vm_get_str(vm);
            BppString *s = str_create(str_ctx, val_buf, len);
            BValue val;
            memset(&val, 0, sizeof(val));
            val.type = VAL_STRING;
            val.as.string = s;
            var_assign(vc, var_name, val);
            str_release(str_ctx, s);
        } else {
            double num = strtod(item_src, NULL);
            BValue val;
            memset(&val, 0, sizeof(val));
            val.type = VAL_NUMBER;
            val.as.number = num;
            var_assign(vc, var_name, val);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_restore_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        vm_set_data_ptr(vm, 0);
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type == VAL_NUMBER) {
        double line_num = val.as.number;
        int count = vm_get_data_count(vm);
        BppDataPosition *items = vm_get_data_items(vm);
        int target_ptr = 0;
        if (items) {
            for (int i = 0; i < count; ++i) {
                if (items[i].line >= line_num) {
                    target_ptr = i;
                    break;
                }
            }
        }
        vm_set_data_ptr(vm, target_ptr);
    } else {
        vm_set_data_ptr(vm, 0);
    }

    return err;
}
