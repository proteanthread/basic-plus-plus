// FILENAME: margin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, margin.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MARGIN statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/margin.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_margin_desc = {
    .name = "MARGIN",
    .category = "Input / Output",
    .syntax = "MARGIN [#channel,] width",
    .description = "Sets line length / print margin width before auto-wrapping output (SDS 940 / DEC PDP-10 Super BASIC).",
    .error_summary = "Error 2: Syntax error, Error 5: Illegal function call, Error 52: Bad file number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_margin_register(void) {
    lang_desc_register(&g_margin_desc);
}

BppError stmt_margin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_MARGIN) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        err.code = ERR_SYNTAX;
        err.message = "Expected width argument in MARGIN statement";
        return err;
    }

    int channel = -1;
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        channel = (int)ch_val.as.number;

        BppToken comma = lex_next(lex);
        if (comma.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected ',' after channel in MARGIN statement";
            return err;
        }
    }

    BValue width_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (width_val.type != VAL_NUMBER && width_val.type != VAL_INTEGER) {
        if (width_val.type == VAL_STRING && width_val.as.string) str_release(vm_get_str(vm), width_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    int width = (int)width_val.as.number;
    if (width < 1 || width > 65535) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "Illegal function call: MARGIN width must be 1 to 65535";
        return err;
    }

    if (channel < 0) {
        vm_set_margin(vm, width);
    } else {
        if (!file_is_open(vm_get_file(vm), channel)) {
            err.code = ERR_BAD_FILE_NUMBER;
            return err;
        }
    }

    return err;
}
