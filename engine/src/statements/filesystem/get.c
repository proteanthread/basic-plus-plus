/**
 * @file get.c
 * @brief GET #file_num [, record_num] random-access and binary file record read handler for BASIC++.
 */
#include "statements/filesystem/get.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_get_register(void) {
    static const MicroLibMetadata meta = {
        .name = "GET",
        .category = "Filesystem I/O",
        .syntax = "GET [#]file_num [, record_number]",
        .help_text = "Reads a record from a random-access file into the FIELD buffer.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}

BppError stmt_get_handler(VMContext *vm, LexerContext *lex) {
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

    long record_num = -1;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (rec_val.type == VAL_NUMBER) {
            record_num = (long)rec_val.as.number;
        }
    }

    int rec_len = file_get_record_len(fc, channel);
    if (rec_len <= 0) rec_len = 128;

    if (record_num > 0) {
        file_seek(fc, channel, (record_num - 1) * rec_len);
    }

    unsigned char *rec_buf = file_get_record_buffer(fc, channel);
    if (rec_buf) {
        file_read(fc, channel, rec_buf, rec_len);
    }

    return err;
}
