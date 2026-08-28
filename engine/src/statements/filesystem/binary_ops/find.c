// FILENAME: find.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, find.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FIND statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/find.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "platform/platform.h"
#include <string.h>

void stmt_find_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FIND",
        .category = "Filesystem I/O",
        .syntax = "FIND [#]channel [, RECORD record_number]",
        .help_text = "DEC RSTS/E RMS-11 statement to position the file pointer at a specific record without data transfer.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}

BppError stmt_find_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken peek = lex_peek(lex);
    if (peek.type == TOK_HASH) {
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

    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    long record_num = -1;
    BppToken comma = lex_peek(lex);
    if (comma.type == TOK_COMMA) {
        lex_next(lex); // Consume ','

        BppToken rec_kw = lex_peek(lex);
        if ((rec_kw.type == TOK_KEYWORD && rec_kw.as.keyword == KW_RECORD) ||
            (rec_kw.type == TOK_IDENT && rec_kw.length == 6 && platform_strncasecmp(rec_kw.start, "RECORD", 6) == 0)) {
            lex_next(lex); // Consume RECORD
        }

        BValue rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (rec_val.type == VAL_NUMBER || rec_val.type == VAL_INTEGER) {
            record_num = (long)rec_val.as.number;
        } else {
            if (rec_val.type == VAL_STRING && rec_val.as.string) str_release(vm_get_str(vm), rec_val.as.string);
        }
    }

    int rec_len = file_get_record_len(fc, channel);
    if (rec_len <= 0) rec_len = 128;

    if (record_num > 0) {
        file_seek(fc, channel, (record_num - 1) * rec_len);
    }

    return err;
}
