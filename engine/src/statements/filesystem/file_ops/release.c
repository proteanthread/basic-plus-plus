// FILENAME: release.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, release.h, vm.h)
// Provides runtime implementation for the RELEASE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/release.h"
#include "statements/filesystem/file_ops/record_lock.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "eval/eval.h"
#include "types/errors.h"

static const LangDesc g_release_desc = {
    .name = "RELEASE",
    .category = "Pick & Business BASIC",
    .syntax = "RELEASE | RELEASE [#]ch | RELEASE [#]ch, id | RELEASE[ch, id]",
    .description = "Releases record locks across all channels, a specific channel, or an individual record.",
    .error_summary = "Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_release_register(void) {
    lang_desc_register(&g_release_desc);
}

BppError stmt_release_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool in_bracket = false;

    // Bare RELEASE: end of line or colon statement separator
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        record_lock_release_all();
        return err;
    }

    if (tok.type == TOK_LBRACKET) {
        lex_next(lex);
        in_bracket = true;
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int channel = (int)ch_val.as.number;

    tok = lex_peek(lex);
    if (in_bracket && tok.type == TOK_RBRACKET) {
        lex_next(lex);
        record_lock_release_channel(channel);
        return err;
    }

    if (tok.type == TOK_COMMA) {
        lex_next(lex); // Consume comma

        BValue id_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        char id_str[64] = {0};
        if (id_val.type == VAL_STRING && id_val.as.string) {
            const char *s = str_data(id_val.as.string);
            if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
            str_release(vm_get_str(vm), id_val.as.string);
        } else {
            runtime_snprintf(id_str, sizeof(id_str), "%g", id_val.as.number);
        }

        if (in_bracket) {
            tok = lex_peek(lex);
            if (tok.type == TOK_RBRACKET) {
                lex_next(lex);
            }
        }

        record_lock_release(channel, id_str);
    } else {
        // Channel-only release
        record_lock_release_channel(channel);
    }

    return err;
}
