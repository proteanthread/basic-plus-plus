// FILENAME: close.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (close.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CLOSE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/close.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_close_desc = {
    .name = "CLOSE",
    .category = "Filesystem I/O",
    .syntax = "CLOSE [[#]file_num1[, [#]file_num2...]]",
    .description = "Closes open file channels, flushing pending I/O buffers to disk.",
    .error_summary = "Error 2: Syntax Error, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_close_register(void) {
    lang_desc_register(&g_close_desc);
}

BppError stmt_close_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
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

        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
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
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    FileContext *fc = vm_get_file(vm);
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int ch = (int)ch_val.as.number;
        file_seek(fc, ch, 0);
        return err;
    }
    file_close_all(fc);
    return err;
}
