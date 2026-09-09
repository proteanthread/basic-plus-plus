// FILENAME: writeu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, vm.h, writeu.h)
// Provides runtime implementation for the WRITEU statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/writeu.h"
#include "statements/filesystem/file_ops/record_lock.h"
#include "statements/db/isam/isam.h"
#include "runtime/language_descriptor.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/file.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "eval/eval.h"
#include "types/errors.h"

static const LangDesc g_writeu_desc = {
    .name = "WRITEU",
    .category = "Pick & Business BASIC",
    .syntax = "WRITEU [#]ch, id, data$ | WRITEU[ch, id] data$ | WRITEU data$ ON [#]ch, id",
    .description = "Writes a record to a channel or ISAM store while retaining the exclusive update lock.",
    .error_summary = "Error 52: Bad File Number, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_writeu_register(void) {
    lang_desc_register(&g_writeu_desc);
}

BppError stmt_writeu_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool in_bracket = false;
    int channel = 0;
    char id_str[64] = {0};
    BValue data_val;
    data_val.type = VAL_NONE;
    data_val.as.number = 0.0;

    // Check if starts with data expression followed by ON / TO
    // Peek if first token is string or expr and not bracket/hash/number
    if (tok.type == TOK_LBRACKET) {
        lex_next(lex);
        in_bracket = true;
        tok = lex_peek(lex);
    }

    if (in_bracket || tok.type == TOK_HASH || tok.type == TOK_NUMBER) {
        if (tok.type == TOK_HASH) lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        channel = (int)ch_val.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected comma after channel in WRITEU";
            return err;
        }

        BValue id_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
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

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
        }

        data_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
    } else {
        // Pick form: WRITEU data$ ON/TO [#]ch, id
        data_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        // Expect 'ON' or 'TO'
        if ((tok.type != TOK_KEYWORD && tok.type != TOK_IDENT) ||
            (runtime_strncasecmp(tok.start, "ON", 2) != 0 && runtime_strncasecmp(tok.start, "TO", 2) != 0)) {
            err.code = ERR_SYNTAX;
            err.message = "Expected ON or TO after data in WRITEU";
            if (data_val.type == VAL_STRING && data_val.as.string) str_release(vm_get_str(vm), data_val.as.string);
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_HASH) lex_next(lex);

        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (data_val.type == VAL_STRING && data_val.as.string) str_release(vm_get_str(vm), data_val.as.string);
            return err;
        }
        channel = (int)ch_val.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected comma before record id in WRITEU";
            if (data_val.type == VAL_STRING && data_val.as.string) str_release(vm_get_str(vm), data_val.as.string);
            return err;
        }

        BValue id_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (data_val.type == VAL_STRING && data_val.as.string) str_release(vm_get_str(vm), data_val.as.string);
            return err;
        }
        if (id_val.type == VAL_STRING && id_val.as.string) {
            const char *s = str_data(id_val.as.string);
            if (s) runtime_strncpy(id_str, s, sizeof(id_str) - 1);
            str_release(vm_get_str(vm), id_val.as.string);
        } else {
            runtime_snprintf(id_str, sizeof(id_str), "%g", id_val.as.number);
        }
    }

    // Retain exclusive update lock
    record_lock_acquire(channel, id_str);

    // Write record to ISAM table or virtual record
    if (isam_is_active(channel)) {
        isam_write_record(vm, channel, id_str, data_val);
    } else {
        if (data_val.type == VAL_STRING && data_val.as.string) {
            const char *ds = str_data(data_val.as.string);
            record_lock_store(channel, id_str, ds ? ds : "");
            str_release(vm_get_str(vm), data_val.as.string);
        } else {
            char num_buf[64] = {0};
            runtime_snprintf(num_buf, sizeof(num_buf), "%g", data_val.as.number);
            record_lock_store(channel, id_str, num_buf);
        }
    }

    return err;
}
