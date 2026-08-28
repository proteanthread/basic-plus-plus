// FILENAME: mat_input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mat_input.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the MAT_INPUT statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/io/mat_input.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_mat_input_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAT INPUT",
        .category = "Matrix Operations",
        .syntax = "MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])]",
        .help_text = "Reads numeric or string matrix elements from console input or an open file stream (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number, Error 62: Input Past End"
    };
    microlib_register(&meta);
}

static bool read_field_from_file(FileContext *fc, int channel, char *buf, size_t max_len) {
    size_t len = 0;
    int c;

    // Skip leading whitespace, newlines, and commas
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

BppError stmt_mat_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    FileContext *fc = vm_get_file(vm);
    ArrayContext *arr = vm_get_arr(vm);
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
        err.message = "Expected array name in MAT INPUT";
        return err;
    }

    char arr_name[64];
    size_t arr_len = (tok.length < sizeof(arr_name) - 1) ? tok.length : sizeof(arr_name) - 1;
    memcpy(arr_name, tok.start, arr_len);
    arr_name[arr_len] = '\0';

    // Optional redimension dimensions: A(r, c) or A(r)
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
                err.message = "Expected ')' after matrix dimensions in MAT INPUT";
                return err;
            }
            int b[2] = {d1, d2};
            if (arr_exists(arr, arr_name)) arr_erase(arr, arr_name);
            arr_dim(arr, arr_name, 2, b);
        } else {
            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = ERR_SYNTAX;
                err.message = "Expected ')' after vector dimension in MAT INPUT";
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

    char field_buf[256];
    if (dims == 1) {
        for (int i = base; i <= bounds[0]; i++) {
            if (channel >= 0) {
                if (!read_field_from_file(fc, channel, field_buf, sizeof(field_buf))) {
                    err.code = ERR_INPUT_PAST_END;
                    return err;
                }
            } else {
                if (vdev) {
                    if (!vdev_gets(vdev, field_buf, sizeof(field_buf))) {
                        field_buf[0] = '\0';
                    }
                    size_t flen = strlen(field_buf);
                    while (flen > 0 && (field_buf[flen - 1] == '\r' || field_buf[flen - 1] == '\n')) {
                        field_buf[--flen] = '\0';
                    }
                }
            }
            int idx = i;
            BValue *elem = arr_get_element(arr, arr_name, 1, &idx, &err);
            if (elem) {
                if (is_str) {
                    if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                    elem->type = VAL_STRING;
                    elem->as.string = str_create(vm_get_str(vm), field_buf, strlen(field_buf));
                } else {
                    elem->type = VAL_NUMBER;
                    elem->as.number = strtod(field_buf, NULL);
                }
            }
        }
    } else if (dims == 2) {
        for (int r = base; r <= bounds[0]; r++) {
            for (int c = base; c <= bounds[1]; c++) {
                if (channel >= 0) {
                    if (!read_field_from_file(fc, channel, field_buf, sizeof(field_buf))) {
                        err.code = ERR_INPUT_PAST_END;
                        return err;
                    }
                } else {
                    if (vdev) {
                        if (!vdev_gets(vdev, field_buf, sizeof(field_buf))) {
                            field_buf[0] = '\0';
                        }
                        size_t flen = strlen(field_buf);
                        while (flen > 0 && (field_buf[flen - 1] == '\r' || field_buf[flen - 1] == '\n')) {
                            field_buf[--flen] = '\0';
                        }
                    }
                }
                int idxs[2] = {r, c};
                BValue *elem = arr_get_element(arr, arr_name, 2, idxs, &err);
                if (elem) {
                    if (is_str) {
                        if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                        elem->type = VAL_STRING;
                        elem->as.string = str_create(vm_get_str(vm), field_buf, strlen(field_buf));
                    } else {
                        elem->type = VAL_NUMBER;
                        elem->as.number = strtod(field_buf, NULL);
                    }
                }
            }
        }
    }

    return err;
}
