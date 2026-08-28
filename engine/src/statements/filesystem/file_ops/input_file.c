// FILENAME: input_file.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (line.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the INPUT_FILE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/input_file.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "device/vdev.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_input_file_register(void) {
    static const MicroLibMetadata meta = {
        .name = "INPUT#",
        .category = "Filesystem I/O",
        .syntax = "INPUT #file_num, var1 [, var2...] | LINE INPUT [#file_num,] string_var",
        .help_text = "Reads data items or full line text strings from an open sequential disk file channel or console.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End"
    };
    microlib_register(&meta);
}

BppError stmt_input_file_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int channel = (int)ch_val.as.number;
    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    VariableContext *vc = vm_get_var(vm);
    ArrayContext *arr_ctx = vm_get_arr(vm);
    StringContext *str_ctx = vm_get_str(vm);

    while (true) {
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Syntax error in INPUT# (expected variable name)";
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
                if (err.code != 0) return err;
                if (idx_val.type != VAL_NUMBER) {
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

        if (file_eof(fc, channel)) {
            err.code = 62;
            err.message = "Input Past End";
            return err;
        }

        char field_buf[1024] = {0};
        size_t len = 0;
        int c;

        while ((c = file_getc(fc, channel)) != -1 && isspace(c) && c != '\n' && c != '\r');

        if (c == -1) {
            err.code = 62;
            err.message = "Input Past End";
            return err;
        }

        if (c == '"') {
            while ((c = file_getc(fc, channel)) != -1 && c != '"' && len < sizeof(field_buf) - 1) {
                field_buf[len++] = (char)c;
            }
            file_getc(fc, channel);
        } else {
            field_buf[len++] = (char)c;
            while ((c = file_getc(fc, channel)) != -1 && c != ',' && c != '\n' && c != '\r' && len < sizeof(field_buf) - 1) {
                field_buf[len++] = (char)c;
            }
            if (c != ',' && c != -1 && c != '\n') {
                file_ungetc(fc, channel, c);
            }
        }
        field_buf[len] = '\0';


        bool is_str = (var_name[strlen(var_name) - 1] == '$');
        BValue val;
        memset(&val, 0, sizeof(val));

        if (is_str) {
            val.type = VAL_STRING;
            val.as.string = str_create(str_ctx, field_buf, len);
        } else {
            double num = strtod(field_buf, NULL);
            val.type = VAL_NUMBER;
            val.as.number = num;
        }

        if (is_array) {
            BValue *target = arr_get_element(arr_ctx, var_name, num_dims, indices, &err);
            if (err.code != 0 || !target) {
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(str_ctx, val.as.string);
                }
                return err;
            }
            if (target->type == VAL_STRING && target->as.string) {
                str_release(str_ctx, target->as.string);
            }
            *target = val;
            if (val.type == VAL_STRING && val.as.string) {
                str_add_ref(val.as.string);
            }
        } else {
            if (!var_assign(vc, var_name, val)) {
                err.code = 13;
                err.message = "Type Mismatch in variable assignment";
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(str_ctx, val.as.string);
                }
                return err;
            }
        }

        if (val.type == VAL_STRING && val.as.string) {
            str_release(str_ctx, val.as.string);
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

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex) {
    return stmt_input_file_handler(vm, lex);
}


