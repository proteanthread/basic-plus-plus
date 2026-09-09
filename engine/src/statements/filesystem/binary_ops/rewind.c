// FILENAME: rewind.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, rewind.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the REWIND statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/rewind.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_rewind_desc = {
    .name = "REWIND",
    .category = "File System & I/O",
    .syntax = "REWIND [#]channel",
    .description = "Rewinds the read/write pointer to the beginning of the specified file channel (HP 2000 TSB).",
    .error_summary = "Error 2: Syntax error, Error 13: Type mismatch, Error 52: Bad file number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_rewind_register(void) {
    lang_desc_register(&g_rewind_desc);
}

BppError stmt_rewind_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_REWIND) {
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

    file_seek(fc, channel, 1);
    return err;
}
