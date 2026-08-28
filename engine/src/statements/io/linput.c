// FILENAME: linput.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, linput.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the LINPUT statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/linput.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "device/vdev.h"
#include <string.h>

void stmt_linput_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LINPUT",
        .category = "Input / Output",
        .syntax = "LINPUT [#channel,] [\"prompt\";] string_var",
        .help_text = "Reads an entire line of text into a string variable without delimiters (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End"
    };
    microlib_register(&meta);
}

BppError stmt_linput_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_LINPUT) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    FileContext *fc = vm_get_file(vm);
    bool is_file = false;
    int channel = -1;

    if (tok.type == TOK_HASH) {
        is_file = true;
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
            err.message = "Bad File Number";
            return err;
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
    } else {
        if (tok.type == TOK_SEMICOLON) {
            lex_next(lex); // Consume leading semicolon (suppress newline)
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_STRING) {
            lex_next(lex);
            const char *prompt = tok.as.string;
            size_t prompt_len = tok.length;
            char prompt_buf[512] = "";
            size_t copy_len = (prompt_len < sizeof(prompt_buf) - 1) ? prompt_len : sizeof(prompt_buf) - 1;
            memcpy(prompt_buf, prompt, copy_len);
            prompt_buf[copy_len] = '\0';

            tok = lex_next(lex);
            if (tok.type != TOK_SEMICOLON && tok.type != TOK_COMMA) {
                err.code = ERR_SYNTAX;
                err.message = "Expected ';' or ',' after prompt in LINPUT";
                return err;
            }
            if (vdev) vdev_puts(vdev, prompt_buf);
            tok = lex_peek(lex);
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = ERR_SYNTAX;
        err.message = "Syntax error in LINPUT (expected variable name)";
        return err;
    }

    char var_name[64];
    size_t var_len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.start, var_len);
    var_name[var_len] = '\0';

    char line_buf[1024] = "";
    if (is_file) {
        if (!file_gets(fc, channel, line_buf, sizeof(line_buf))) {
            err.code = ERR_INPUT_PAST_END;
            err.message = "Input past end of file";
            return err;
        }
    } else {
        if (vdev) {
            if (!vdev_gets(vdev, line_buf, sizeof(line_buf))) {
                line_buf[0] = '\0';
            }
        }
    }

    // Remove trailing \r or \n
    size_t llen = strlen(line_buf);
    while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
        line_buf[--llen] = '\0';
    }

    BValue str_val;
    memset(&str_val, 0, sizeof(str_val));
    str_val.type = VAL_STRING;
    str_val.as.string = str_create(vm_get_str(vm), line_buf, llen);

    var_assign(vm_get_var(vm), var_name, str_val);
    str_release(vm_get_str(vm), str_val.as.string);

    return err;
}
