/**
 * @file print_file.c
 * @brief PRINT #file_num, expr [, ...] formatted file output statement handler for BASIC++.
 */
#include "statements/filesystem/print_file.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_print_file_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PRINT#",
        .category = "Filesystem I/O",
        .syntax = "PRINT #file_num, expression_list",
        .help_text = "Writes sequential formatted text data to an open disk file channel.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 54: Bad File Mode"
    };
    microlib_register(&meta);
}

BppError stmt_print_file_handler(VMContext *vm, LexerContext *lex) {
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

    bool trailing_sep = false;

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (val.type == VAL_NUMBER) {
            file_printf(fc, channel, " %g ", val.as.number);
        } else if (val.type == VAL_STRING && val.as.string) {
            file_puts(fc, channel, str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA || tok.type == TOK_SEMICOLON) {
            trailing_sep = true;
            if (tok.type == TOK_COMMA) file_puts(fc, channel, "\t");
            lex_next(lex);
        } else {
            trailing_sep = false;
        }
    }

    if (!trailing_sep) {
        file_puts(fc, channel, "\n");
    }

    return err;
}

BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex) {
    return stmt_print_file_handler(vm, lex);
}
