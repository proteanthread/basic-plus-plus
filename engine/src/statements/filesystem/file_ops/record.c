// FILENAME: record.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (class.h, class.c, eval.h, eval.c, record.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the RECORD statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/record.h"
#include "statements/oop/structure/class.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_record_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RECORD",
        .category = "Filesystem I/O",
        .syntax = "RECORD [#]channel, record_number | RECORD record_name ... END RECORD",
        .help_text = "Sets the next record pointer for direct file I/O (DEC PDP-10), or defines a structured record type (VAX BASIC).",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}

BppError stmt_record_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken peek = lex_peek(lex);
    if (peek.type == TOK_IDENT) {
        return stmt_record_type_handler(vm, lex);
    }

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

    BppToken comma = lex_next(lex);
    if (comma.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after channel number in RECORD statement";
        return err;
    }

    BValue rec_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (rec_val.type != VAL_NUMBER && rec_val.type != VAL_INTEGER) {
        if (rec_val.type == VAL_STRING && rec_val.as.string) str_release(vm_get_str(vm), rec_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    long rec_num = (long)rec_val.as.number;
    if (rec_num < 1) rec_num = 1;

    // Seek channel to record offset (128 bytes per record standard)
    file_seek(vm_get_file(vm), channel, (rec_num - 1) * 128);
    return err;
}
