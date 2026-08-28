// FILENAME: mat_print.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h, mat_write.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the MAT_PRINT statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/io/mat_print.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/num_format.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>
#include <stdio.h>

void stmt_mat_print_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAT PRINT",
        .category = "Matrix Operations",
        .syntax = "MAT PRINT [#file_num,] array_name [;|,]",
        .help_text = "Outputs formatted 1D or 2D matrix array elements to console or file stream (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_mat_print_handler(VMContext *vm, LexerContext *lex) {
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
        err.message = "Expected array name in MAT PRINT";
        return err;
    }

    char arr_name[64];
    size_t arr_len = (tok.length < sizeof(arr_name) - 1) ? tok.length : sizeof(arr_name) - 1;
    memcpy(arr_name, tok.start, arr_len);
    arr_name[arr_len] = '\0';

    bool is_compact = false;
    BppToken sep = lex_peek(lex);
    if (sep.type == TOK_SEMICOLON) {
        is_compact = true;
        lex_next(lex);
    } else if (sep.type == TOK_COMMA) {
        is_compact = false;
        lex_next(lex);
    }

    if (!arr_exists(arr, arr_name)) {
        err.code = ERR_SUBSCRIPT_OUT_OF_RANGE;
        err.message = "Array not dimensioned in MAT PRINT";
        return err;
    }

    int bounds[4] = {0};
    int dims = arr_get_dimensions(arr, arr_name, bounds, 4);
    int base = arr_get_option_base(arr);
    size_t zone_w = (size_t)vm_get_zone_width(vm);
    if (zone_w == 0) zone_w = 14;

    if (dims == 1) {
        for (int i = base; i <= bounds[0]; i++) {
            int idx = i;
            BValue *elem = arr_get_element(arr, arr_name, 1, &idx, &err);
            char buf[128] = "";
            if (elem) {
                if (elem->type == VAL_STRING && elem->as.string) {
                    snprintf(buf, sizeof(buf), "%s", str_data(elem->as.string));
                } else {
                    num_format_display(buf, sizeof(buf), elem->as.number, false, false);
                }
            }
            if (channel >= 0) {
                file_puts(fc, channel, buf);
                if (is_compact) file_puts(fc, channel, " ");
                else {
                    size_t pad = (strlen(buf) < zone_w) ? (zone_w - strlen(buf)) : 1;
                    for (size_t s = 0; s < pad; s++) file_putc(fc, channel, ' ');
                }
            } else {
                if (vdev) {
                    vdev_puts(vdev, buf);
                    if (is_compact) vdev_puts(vdev, " ");
                    else {
                        size_t pad = (strlen(buf) < zone_w) ? (zone_w - strlen(buf)) : 1;
                        for (size_t s = 0; s < pad; s++) vdev_putc(vdev, ' ');
                    }
                }
            }
        }
        if (channel >= 0) file_putc(fc, channel, '\n');
        else if (vdev) vdev_putc(vdev, '\n');
    } else if (dims == 2) {
        for (int r = base; r <= bounds[0]; r++) {
            for (int c = base; c <= bounds[1]; c++) {
                int idxs[2] = {r, c};
                BValue *elem = arr_get_element(arr, arr_name, 2, idxs, &err);
                char buf[128] = "";
                if (elem) {
                    if (elem->type == VAL_STRING && elem->as.string) {
                        snprintf(buf, sizeof(buf), "%s", str_data(elem->as.string));
                    } else {
                        num_format_display(buf, sizeof(buf), elem->as.number, false, false);
                    }
                }
                if (channel >= 0) {
                    file_puts(fc, channel, buf);
                    if (is_compact) file_puts(fc, channel, " ");
                    else {
                        size_t pad = (strlen(buf) < zone_w) ? (zone_w - strlen(buf)) : 1;
                        for (size_t s = 0; s < pad; s++) file_putc(fc, channel, ' ');
                    }
                } else {
                    if (vdev) {
                        vdev_puts(vdev, buf);
                        if (is_compact) vdev_puts(vdev, " ");
                        else {
                            size_t pad = (strlen(buf) < zone_w) ? (zone_w - strlen(buf)) : 1;
                            for (size_t s = 0; s < pad; s++) vdev_putc(vdev, ' ');
                        }
                    }
                }
            }
            if (channel >= 0) file_putc(fc, channel, '\n');
            else if (vdev) vdev_putc(vdev, '\n');
        }
    }

    return err;
}
