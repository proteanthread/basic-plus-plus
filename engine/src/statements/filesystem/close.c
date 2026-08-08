/**
 * @file close.c
 * @brief CLOSE file handle statement handler for BASIC++.
 */
#include "statements/filesystem/close.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_close_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CLOSE",
        .category = "Filesystem I/O",
        .syntax = "CLOSE [[#]file_num1[, [#]file_num2...]]",
        .help_text = "Closes open file channels, flushing pending I/O buffers to disk.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_close_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    FileContext *fc = vm_get_file(vm);

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        file_close_all(fc);
        return err;
    }

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_HASH) {
            lex_next(lex);
        }

        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (val.type == VAL_NUMBER) {
            int channel = (int)val.as.number;
            file_close(fc, channel);
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

BppError stmt_reset_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    FileContext *fc = vm_get_file(vm);
    file_close_all(fc);
    return err;
}
