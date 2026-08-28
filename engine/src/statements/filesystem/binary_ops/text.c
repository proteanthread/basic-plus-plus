// FILENAME: text.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, string.c, text.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the TEXT statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/text.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_text_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TEXT",
        .category = "Filesystem I/O",
        .syntax = "TEXT [#]channel, \"filespec\"",
        .help_text = "Opens a file channel in sequential text output mode (SDS 940 / DEC PDP-10 Super BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 55: File Already Open, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

BppError stmt_text_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TEXT) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int channel = (int)ch_val.as.number;

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after channel number in TEXT";
        return err;
    }

    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        err.message = "Expected string filename in TEXT";
        return err;
    }

    err = file_open(vm_get_file(vm), vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                    FILE_MODE_OUTPUT, FILE_ACCESS_WRITE, FILE_LOCK_DEFAULT, 128);
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}
