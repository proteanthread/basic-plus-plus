// FILENAME: line_input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, line_input.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the LINE_INPUT statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/io/line_input.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

void stmt_line_input_register(void) {
    MicroLibMetadata meta = {
        .name = "LINE INPUT",
        .category = "Console & File I/O",
        .syntax = "LINE INPUT [;] [\"prompt\";] string_var$ | LINE INPUT #file_num%, string_var$",
        .help_text = "Reads an entire line of up to 255 characters from the keyboard or a file into a string variable.",
        .error_codes = "Error 2: Syntax Error, Error 5: Failed to read input, Error 52: Bad File Number, Error 62: Input Past End"
    };
    microlib_register(&meta);
}

BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_file = false;
    int channel = 0;
    FileContext *fc = vm_get_file(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    VariableContext *vc = vm_get_var(vm);
    ArrayContext *arr_ctx = vm_get_arr(vm);
    StringContext *str_ctx = vm_get_str(vm);

    if (tok.type == TOK_HASH) {
        is_file = true;
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        channel = (int)ch_val.as.number;
        if (!file_is_open(fc, channel)) {
            err.code = 52;
            err.message = "Bad File Number";
            return err;
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        }
    } else {
        tok = lex_peek(lex);
        if (tok.type == TOK_SEMICOLON) {
            lex_next(lex); // Consume optional leading semicolon
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_STRING) {
            // Console prompt string: LINE INPUT "prompt"; var$
            lex_next(lex);
            const char *prompt = tok.as.string;
            size_t prompt_len = tok.length;
            char prompt_buf[512] = "";
            size_t copy_len = (prompt_len < sizeof(prompt_buf) - 1) ? prompt_len : sizeof(prompt_buf) - 1;
            memcpy(prompt_buf, prompt, copy_len);
            prompt_buf[copy_len] = '\0';

            tok = lex_next(lex);
            if (tok.type != TOK_SEMICOLON && tok.type != TOK_COMMA) {
                err.code = 2; err.message = "Expected ';' or ',' after prompt in LINE INPUT";
                return err;
            }
            vdev_puts(vdev, prompt_buf);
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Syntax error in LINE INPUT (expected variable name)";
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

    char line_buf[2048] = {0};

    if (is_file) {
        if (file_eof(fc, channel)) {
            err.code = 62;
            err.message = "Input Past End";
            return err;
        }
        if (file_gets(fc, channel, line_buf, sizeof(line_buf))) {
            size_t l = strlen(line_buf);
            while (l > 0 && (line_buf[l - 1] == '\r' || line_buf[l - 1] == '\n')) {
                line_buf[--l] = '\0';
            }
        }
    } else {
        VDev *con_dev = vdev_get(vdev, "CON:");
        if (con_dev && con_dev->ops.flush) {
            con_dev->ops.flush(con_dev);
        }
        bool read_ok = false;
        if (con_dev && con_dev->ops.gets) {
            read_ok = con_dev->ops.gets(con_dev, line_buf, sizeof(line_buf));
        }
        if (!read_ok) {
            if (fgets(line_buf, sizeof(line_buf), stdin)) {
                read_ok = true;
            }
        }
        if (!read_ok) {
            err.code = 5; err.message = "Failed to read input from console device";
            return err;
        }
        size_t l = strlen(line_buf);
        while (l > 0 && (line_buf[l - 1] == '\r' || line_buf[l - 1] == '\n')) {
            line_buf[--l] = '\0';
        }
    }

    BppString *s = str_create(str_ctx, line_buf, strlen(line_buf));
    BValue val;
    memset(&val, 0, sizeof(val));
    val.type = VAL_STRING;
    val.as.string = s;

    if (is_array) {
        BValue *target = arr_get_element(arr_ctx, var_name, num_dims, indices, &err);
        if (err.code != 0 || !target) {
            str_release(str_ctx, s);
            return err;
        }
        if (target->type == VAL_STRING && target->as.string) {
            str_release(str_ctx, target->as.string);
        }
        *target = val;
        str_add_ref(s);
    } else {
        if (!var_assign(vc, var_name, val)) {
            err.code = 13;
            err.message = "Type Mismatch in variable assignment";
            str_release(str_ctx, s);
            return err;
        }
    }

    str_release(str_ctx, s);
    return err;
}
