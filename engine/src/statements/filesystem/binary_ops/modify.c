// FILENAME: modify.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, modify.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MODIFY statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/modify.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_modify_desc = {
    .name = "MODIFY",
    .category = "Filesystem I/O",
    .syntax = "MODIFY #channel, \"field_spec\"",
    .description = "Opens a file channel in read/write random access modify mode (SDS 940 / DEC PDP-10 Super BASIC).",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 55: File Already Open, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_modify_register(void) {
    lang_desc_register(&g_modify_desc);
}

BppError stmt_modify_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_MODIFY) {
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
        err.message = "Expected ',' after channel number in MODIFY";
        return err;
    }

    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        err.message = "Expected string filename in MODIFY";
        return err;
    }

    err = file_open(vm_get_file(vm), vm_get_vdev(vm), channel, str_data(fn_val.as.string),
                    FILE_MODE_RANDOM, FILE_ACCESS_READ_WRITE, FILE_LOCK_DEFAULT, 128);
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}
