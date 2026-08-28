// FILENAME: backspace.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (backspace.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the BACKSPACE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/backspace.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_backspace_register(void) {
    static const MicroLibMetadata meta = {
        .name = "BACKSPACE",
        .category = "File System & I/O",
        .syntax = "BACKSPACE [#]channel",
        .help_text = "Repositions the file pointer backwards by one record or block (IBM CALL/360 / DG).",
        .error_codes = "Error 2: Syntax error, Error 13: Type mismatch, Error 52: Bad file number"
    };
    microlib_register(&meta);
}

BppError stmt_backspace_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_BACKSPACE) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int channel = (int)ch_val.as.number;
    FileContext *fc = vm_get_file(vm);
    if (!fc || !file_is_open(fc, channel)) {
        err.code = ERR_BAD_FILE_NUMBER;
        return err;
    }

    long cur_pos = file_loc(fc, channel);
    int rlen = file_get_record_len(fc, channel);
    if (rlen <= 0) rlen = 1;

    long new_pos = (cur_pos > rlen) ? (cur_pos - rlen) : 1;
    file_seek(fc, channel, new_pos);
    return err;
}
