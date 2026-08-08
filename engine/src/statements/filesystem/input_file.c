/**
 * @file input_file.c
 * @brief INPUT #file_num, var [, ...] sequential file input statement handler for BASIC++.
 */
#include "statements/filesystem/input_file.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_input_file_register(void) {
    static const MicroLibMetadata meta = {
        .name = "INPUT#",
        .category = "Filesystem I/O",
        .syntax = "INPUT #file_num, var1 [, var2...] | LINE INPUT #file_num, string_var",
        .help_text = "Reads data items or full line text strings from an open sequential disk file channel.",
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

        if (file_eof(fc, channel)) {
            err.code = 62;
            err.message = "Input Past End";
            return err;
        }

        char field_buf[1024] = {0};
        size_t len = 0;
        int c;

        while ((c = file_getc(fc, channel)) != EOF && isspace(c) && c != '\n' && c != '\r');

        if (c == EOF) {
            err.code = 62;
            err.message = "Input Past End";
            return err;
        }

        if (c == '"') {
            while ((c = file_getc(fc, channel)) != EOF && c != '"' && len < sizeof(field_buf) - 1) {
                field_buf[len++] = (char)c;
            }
            file_getc(fc, channel);
        } else {
            field_buf[len++] = (char)c;
            while ((c = file_getc(fc, channel)) != EOF && c != ',' && c != '\n' && c != '\r' && len < sizeof(field_buf) - 1) {
                field_buf[len++] = (char)c;
            }
            if (c != ',' && c != EOF && c != '\n') {
                file_ungetc(fc, channel, c);
            }
        }
        field_buf[len] = '\0';

        bool is_str = (var_name[strlen(var_name) - 1] == '$');
        if (is_str) {
            StringContext *str_ctx = vm_get_str(vm);
            BppString *s = str_create(str_ctx, field_buf, len);
            BValue val;
            memset(&val, 0, sizeof(val));
            val.type = VAL_STRING;
            val.as.string = s;
            var_assign(vc, var_name, val);
            str_release(str_ctx, s);
        } else {
            double num = strtod(field_buf, NULL);
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

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex) {
    return stmt_input_file_handler(vm, lex);
}

BppError stmt_line_input_handler(VMContext *vm, LexerContext *lex) {
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

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2;
        err.message = "Syntax error in LINE INPUT#";
        return err;
    }

    char var_name[64];
    if (tok.length >= sizeof(var_name)) tok.length = sizeof(var_name) - 1;
    memcpy(var_name, tok.start, tok.length);
    var_name[tok.length] = '\0';

    if (file_eof(fc, channel)) {
        err.code = 62;
        err.message = "Input Past End";
        return err;
    }

    char line_buf[2048] = {0};
    if (file_gets(fc, channel, line_buf, sizeof(line_buf))) {
        size_t l = strlen(line_buf);
        while (l > 0 && (line_buf[l - 1] == '\r' || line_buf[l - 1] == '\n')) {
            line_buf[--l] = '\0';
        }
        StringContext *str_ctx = vm_get_str(vm);
        BppString *s = str_create(str_ctx, line_buf, l);
        BValue val;
        memset(&val, 0, sizeof(val));
        val.type = VAL_STRING;
        val.as.string = s;
        var_assign(vm_get_var(vm), var_name, val);
        str_release(str_ctx, s);
    }

    return err;
}
