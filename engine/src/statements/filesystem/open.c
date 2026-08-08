/**
 * @file open.c
 * @brief OPEN file handle statement handler for BASIC++.
 */
#include "statements/filesystem/open.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void stmt_open_register(void) {
    static const MicroLibMetadata meta = {
        .name = "OPEN",
        .category = "Filesystem I/O",
        .syntax = "OPEN filespec [FOR mode] AS [#]file_num [LEN=reclen]",
        .help_text = "Opens a file channel for INPUT, OUTPUT, APPEND, BINARY, or RANDOM I/O operations.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open"
    };
    microlib_register(&meta);
}

BppError stmt_open_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue arg1 = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    char filename[256] = {0};
    char mode_str[16] = {0};
    BppFileMode mode = FILE_MODE_RANDOM;
    int channel = -1;
    int reclen = 128;

    if (arg1.type == VAL_STRING && arg1.as.string) {
        const char *data = str_data(arg1.as.string);
        size_t len = str_len(arg1.as.string);
        if (len < sizeof(mode_str) && (platform_strcasecmp(data, "I") == 0 || platform_strcasecmp(data, "O") == 0 || platform_strcasecmp(data, "A") == 0 || platform_strcasecmp(data, "R") == 0 || platform_strcasecmp(data, "B") == 0)) {
            /* Classic GWBASIC syntax: OPEN "mode", [#]file_num, "filename" [, reclen] */
            snprintf(mode_str, sizeof(mode_str), "%s", data);
            str_release(vm_get_str(vm), arg1.as.string);

            BppToken comma1 = lex_peek(lex);
            if (comma1.type == TOK_COMMA) {
                lex_next(lex);
                BppToken hash = lex_peek(lex);
                if (hash.type == TOK_HASH) {
                    lex_next(lex);
                }
                BValue ch_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (ch_val.type == VAL_NUMBER) channel = (int)ch_val.as.number;

                BppToken comma2 = lex_peek(lex);
                if (comma2.type == TOK_COMMA) {
                    lex_next(lex);
                    BValue fn_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (fn_val.type == VAL_STRING && fn_val.as.string) {
                        snprintf(filename, sizeof(filename), "%s", str_data(fn_val.as.string));
                        str_release(vm_get_str(vm), fn_val.as.string);
                    }
                }
            }
        } else {
            /* Modern QBASIC syntax: OPEN "filename" [FOR mode] AS [#]file_num [LEN=reclen] */
            snprintf(filename, sizeof(filename), "%s", data);
            str_release(vm_get_str(vm), arg1.as.string);

            BppToken tok = lex_peek(lex);
            if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
                char kw[32];
                if (tok.length >= sizeof(kw)) tok.length = sizeof(kw) - 1;
                memcpy(kw, tok.start, tok.length);
                kw[tok.length] = '\0';

                if (platform_strcasecmp(kw, "FOR") == 0) {
                    lex_next(lex);
                    tok = lex_next(lex);
                    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
                        if (tok.length >= sizeof(mode_str)) tok.length = sizeof(mode_str) - 1;
                        memcpy(mode_str, tok.start, tok.length);
                        mode_str[tok.length] = '\0';
                    }
                    tok = lex_peek(lex);
                    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
                        if (tok.length >= sizeof(kw)) tok.length = sizeof(kw) - 1;
                        memcpy(kw, tok.start, tok.length);
                        kw[tok.length] = '\0';
                    }
                }

                if (platform_strcasecmp(kw, "AS") == 0) {
                    lex_next(lex);
                    BppToken hash = lex_peek(lex);
                    if (hash.type == TOK_HASH) {
                        lex_next(lex);
                    }
                    BValue ch_val = eval_expression(vm, lex, &err);
                    if (err.code != 0) return err;
                    if (ch_val.type == VAL_NUMBER) channel = (int)ch_val.as.number;
                }
            }
        }
    }

    if (platform_strcasecmp(mode_str, "I") == 0 || platform_strcasecmp(mode_str, "INPUT") == 0) {
        mode = FILE_MODE_INPUT;
    } else if (platform_strcasecmp(mode_str, "O") == 0 || platform_strcasecmp(mode_str, "OUTPUT") == 0) {
        mode = FILE_MODE_OUTPUT;
    } else if (platform_strcasecmp(mode_str, "A") == 0 || platform_strcasecmp(mode_str, "APPEND") == 0) {
        mode = FILE_MODE_APPEND;
    } else if (platform_strcasecmp(mode_str, "B") == 0 || platform_strcasecmp(mode_str, "BINARY") == 0) {
        mode = FILE_MODE_BINARY;
    } else {
        mode = FILE_MODE_RANDOM;
    }

    if (channel < 1 || channel > 16) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    FileContext *fc = vm_get_file(vm);
    if (file_is_open(fc, channel)) {
        err.code = 55;
        err.message = "File Already Open";
        return err;
    }

    return file_open(fc, vm_get_vdev(vm), channel, filename, mode, FILE_ACCESS_DEFAULT, FILE_LOCK_DEFAULT, reclen);
}
