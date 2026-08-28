// FILENAME: read.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, read.h, string.c, vm.h)
// Provides runtime implementation for the READ statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/data/read.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_read_register(void) {
    static const MicroLibMetadata meta = {
        .name = "READ",
        .category = "Variables & Memory",
        .syntax = "READ var1 [, var2...]",
        .help_text = "Reads sequential values from DATA statements and assigns them to variables.",
        .error_codes = "Error 2: Syntax Error, Error 4: Out of Data, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
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

        bool is_array = false;
        int indices[8];
        int num_dims = 0;

        BppToken peek_sub = lex_peek(lex);
        if (peek_sub.type == TOK_LPAREN) {
            is_array = true;
            lex_next(lex); // Consume '('
            while (num_dims < 8) {
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) {
                    return err;
                }
                if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                    err.code = 13;
                    err.message = "Type Mismatch in array subscript (expected numeric index)";
                    return err;
                }
                indices[num_dims++] = (int)idx_val.as.number;

                peek_sub = lex_peek(lex);
                if (peek_sub.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (peek_sub.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2;
                    err.message = "Syntax error in array subscript (expected ',' or ')')";
                    return err;
                }
            }
        }

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

        bool is_str = (var_name[strlen(var_name) - 1] == '$');
        BValue val;
        memset(&val, 0, sizeof(val));

        if (is_str) {
            char val_buf[256];
            size_t len = 0;
            if (*item_src == '"') {
                item_src++;
                while (*item_src && *item_src != '"' && len < sizeof(val_buf) - 1) {
                    val_buf[len++] = *item_src++;
                }
            } else if (*(item_src - 1) == '"') {
                while (*item_src && *item_src != '"' && len < sizeof(val_buf) - 1) {
                    val_buf[len++] = *item_src++;
                }
            } else {
                while (*item_src && *item_src != ',' && *item_src != ':' && *item_src != '\n' && *item_src != '\r' && len < sizeof(val_buf) - 1) {
                    if (*item_src == '"') break;
                    val_buf[len++] = *item_src++;
                }
                while (len > 0 && (isspace((unsigned char)val_buf[len - 1]) || val_buf[len - 1] == '"')) len--;
            }
            val_buf[len] = '\0';
            StringContext *str_ctx = vm_get_str(vm);
            val.type = VAL_STRING;
            val.as.string = str_create(str_ctx, val_buf, len);
        } else {
            double num = 0.0;
            while (*item_src && isspace((unsigned char)*item_src)) item_src++;
            if (*item_src == '&' && (item_src[1] == 'H' || item_src[1] == 'h')) {
                num = (double)strtoull(item_src + 2, NULL, 16);
            } else if (*item_src == '&' && (item_src[1] == 'O' || item_src[1] == 'o')) {
                num = (double)strtoull(item_src + 2, NULL, 8);
            } else {
                num = strtod(item_src, NULL);
            }
            val.type = VAL_NUMBER;
            val.as.number = num;
        }

        if (is_array) {
            ArrayContext *arr_ctx = vm_get_arr(vm);
            BValue *target = arr_get_element(arr_ctx, var_name, num_dims, indices, &err);
            if (err.code != 0 || !target) {
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }
            if (target->type == VAL_STRING && target->as.string) {
                str_release(vm_get_str(vm), target->as.string);
            }
            *target = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            }
        } else {
            VariableContext *vc = vm_get_var(vm);
            if (!var_assign(vc, var_name, val)) {
                err.code = 13;
                err.message = "Type Mismatch in variable assignment";
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
                return err;
            }
        }

        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }

        BppToken next_tok = lex_peek(lex);
        if (next_tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}
