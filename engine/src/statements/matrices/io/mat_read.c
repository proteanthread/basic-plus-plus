// FILENAME: mat_read.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mat_read.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MAT_READ statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/io/mat_read.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_mat_read_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAT READ",
        .category = "Matrix Operations",
        .syntax = "MAT READ [#channel,] array_name [(num_rows [, num_cols])]",
        .help_text = "Populates matrix elements sequentially from DATA statements or open file stream (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 4: Out of DATA, Error 9: Subscript Out of Range, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

static bool read_field_from_file(FileContext *fc, int channel, char *buf, size_t max_len) {
    size_t len = 0;
    int c;
    while ((c = file_getc(fc, channel)) != -1 && (isspace((unsigned char)c) || c == ','));
    if (c == -1) return false;

    if (c == '"') {
        while ((c = file_getc(fc, channel)) != -1 && c != '"' && len < max_len - 1) {
            buf[len++] = (char)c;
        }
    } else {
        buf[len++] = (char)c;
        while ((c = file_getc(fc, channel)) != -1 && !isspace((unsigned char)c) && c != ',' && len < max_len - 1) {
            buf[len++] = (char)c;
        }
    }
    buf[len] = '\0';
    return (len > 0);

}

BppError stmt_mat_read_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    ArrayContext *arr = vm_get_arr(vm);
    FileContext *fc = vm_get_file(vm);
    int channel = -1;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        channel = (int)ch_val.as.number;
        if (!file_is_open(fc, channel)) {
            err.code = ERR_BAD_FILE_NUMBER;
            return err;
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = ERR_SYNTAX;
        err.message = "Expected array name in MAT READ";
        return err;
    }

    char arr_name[64];
    size_t arr_len = (tok.length < sizeof(arr_name) - 1) ? tok.length : sizeof(arr_name) - 1;
    memcpy(arr_name, tok.start, arr_len);
    arr_name[arr_len] = '\0';

    // Optional redimension: A(r, c) or A(r)
    if (lex_peek(lex).type == TOK_LPAREN) {
        lex_next(lex); // Consume '('
        BValue v1 = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (v1.type == VAL_STRING && v1.as.string) str_release(vm_get_str(vm), v1.as.string);
        int d1 = (int)v1.as.number;

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            BValue v2 = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (v2.type == VAL_STRING && v2.as.string) str_release(vm_get_str(vm), v2.as.string);
            int d2 = (int)v2.as.number;

            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = ERR_SYNTAX;
                err.message = "Expected ')' after matrix dimensions in MAT READ";
                return err;
            }
            int b[2] = {d1, d2};
            if (arr_exists(arr, arr_name)) arr_erase(arr, arr_name);
            arr_dim(arr, arr_name, 2, b);
        } else {
            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = ERR_SYNTAX;
                err.message = "Expected ')' after vector dimension in MAT READ";
                return err;
            }
            int b[1] = {d1};
            if (arr_exists(arr, arr_name)) arr_erase(arr, arr_name);
            arr_dim(arr, arr_name, 1, b);
        }
    }

    if (!arr_exists(arr, arr_name)) {
        int def_b[1] = {10};
        arr_dim(arr, arr_name, 1, def_b);
    }

    int bounds[4] = {0};
    int dims = arr_get_dimensions(arr, arr_name, bounds, 4);
    int base = arr_get_option_base(arr);
    bool is_str = (arr_name[strlen(arr_name) - 1] == '$');

    int ptr = vm_get_data_ptr(vm);
    int count = vm_get_data_count(vm);
    BppDataPosition *items = vm_get_data_items(vm);

    if (dims == 1) {
        for (int i = base; i <= bounds[0]; i++) {
            char val_buf[256] = {0};
            if (channel >= 0) {
                if (!read_field_from_file(fc, channel, val_buf, sizeof(val_buf))) {
                    err.code = ERR_INPUT_PAST_END;
                    err.message = "Input past end in MAT READ";
                    return err;
                }
            } else {
                if (!items || ptr >= count) {
                    err.code = ERR_OUT_OF_DATA;
                    err.message = "Out of DATA in MAT READ";
                    return err;
                }
                const char *item_src = items[ptr++].pos;
                vm_set_data_ptr(vm, ptr);
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
                }
                val_buf[len] = '\0';
            }

            int idx = i;
            BValue *elem = arr_get_element(arr, arr_name, 1, &idx, &err);
            if (elem) {
                if (is_str) {
                    if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                    elem->type = VAL_STRING;
                    elem->as.string = str_create(vm_get_str(vm), val_buf, strlen(val_buf));
                } else {
                    elem->type = VAL_NUMBER;
                    elem->as.number = strtod(val_buf, NULL);
                }
            }
        }
    } else if (dims == 2) {
        for (int r = base; r <= bounds[0]; r++) {
            for (int c = base; c <= bounds[1]; c++) {
                char val_buf[256] = {0};
                if (channel >= 0) {
                    if (!read_field_from_file(fc, channel, val_buf, sizeof(val_buf))) {
                        err.code = ERR_INPUT_PAST_END;
                        err.message = "Input past end in MAT READ";
                        return err;
                    }
                } else {
                    if (!items || ptr >= count) {
                        err.code = ERR_OUT_OF_DATA;
                        err.message = "Out of DATA in MAT READ";
                        return err;
                    }
                    const char *item_src = items[ptr++].pos;
                    vm_set_data_ptr(vm, ptr);
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
                    }
                    val_buf[len] = '\0';
                }

                int idxs[2] = {r, c};
                BValue *elem = arr_get_element(arr, arr_name, 2, idxs, &err);
                if (elem) {
                    if (is_str) {
                        if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                        elem->type = VAL_STRING;
                        elem->as.string = str_create(vm_get_str(vm), val_buf, strlen(val_buf));
                    } else {
                        elem->type = VAL_NUMBER;
                        elem->as.number = strtod(val_buf, NULL);
                    }
                }
            }
        }
    }

    return err;
}
