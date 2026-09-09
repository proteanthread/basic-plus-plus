// FILENAME: write_file.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libengine (write_file.h)
// Provides runtime implementation for the WRITE_FILE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/write_file.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_write_desc = {
    .name = "WRITE#",
    .category = "Filesystem I/O",
    .syntax = "WRITE #file_num, expression_list",
    .description = "Outputs CSV delimited double-quoted strings and formatted numbers to a disk file channel.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number, Error 54: Bad File Mode",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_write_file_register(void) {
    lang_desc_register(&g_write_desc);
}

BppError stmt_write_file_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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

    bool first = true;

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }

        if (!first) {
            file_puts(fc, channel, ",");
        }
        first = false;

        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            file_printf(fc, channel, "%g", val.as.number);
        } else if (val.type == VAL_STRING && val.as.string) {
            file_printf(fc, channel, "\"%s\"", str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    file_puts(fc, channel, "\n");

    return err;
}
