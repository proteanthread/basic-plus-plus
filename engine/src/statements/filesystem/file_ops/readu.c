// FILENAME: readu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, readu.h, vm.h)
// Provides runtime implementation for the READU statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/readu.h"
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

static const LangDesc g_readu_desc = {
    .name = "READU",
    .category = "Pick & Business BASIC",
    .syntax = "READU [#]ch, id, var$ | READU[ch, id] var$ | READU var$ FROM [#]ch, id",
    .description = "Reads a record from a channel or ISAM store and sets an exclusive update lock.",
    .error_summary = "Error 52: Bad File Number, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_readu_register(void) {
    lang_desc_register(&g_readu_desc);
}

BppError stmt_readu_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool in_bracket = false;
    char var_name[64] = {0};
    int channel = 0;
    char id_str[64] = {0};

    // Syntax Form C: READU var$ FROM [#]ch, id
    if (tok.type == TOK_IDENT) {
        size_t vlen = (tok.length < 63) ? tok.length : 63;
        runtime_memcpy(var_name, tok.start, vlen);
        var_name[vlen] = '\0';
        lex_next(lex);

        tok = lex_next(lex);
        // Expect 'FROM' keyword or ident
        if ((tok.type != TOK_KEYWORD && tok.type != TOK_IDENT) ||
            runtime_strncasecmp(tok.start, "FROM", 4) != 0) {
            err.code = ERR_SYNTAX;
            err.message = "Expected FROM after variable in READU";
            return err;
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_HASH) lex_next(lex);

        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        channel = (int)ch_val.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected comma before record id in READU";
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
    } else {
        // Syntax Form A & B: READU [#]ch, id, var$  or  READU[ch, id] var$
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
        channel = (int)ch_val.as.number;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected comma after channel in READU";
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
            tok = lex_peek(lex);
        }

        if (tok.type == TOK_IDENT) {
            size_t vlen = (tok.length < 63) ? tok.length : 63;
            runtime_memcpy(var_name, tok.start, vlen);
            var_name[vlen] = '\0';
            lex_next(lex);
        }
    }

    // Acquire exclusive update lock
    record_lock_acquire(channel, id_str);

    // Read record data
    BValue rec_val;
    rec_val.type = VAL_NONE;
    rec_val.as.number = 0.0;

    if (isam_is_active(channel)) {
        if (!isam_read_record(vm, channel, id_str, &rec_val)) {
            rec_val.type = VAL_STRING;
            rec_val.as.string = str_create(vm_get_str(vm), "", 0);
        }
    } else {
        // Fall back to virtual record lock table
        const char *data_str = record_lock_retrieve(channel, id_str);
        rec_val.type = VAL_STRING;
        rec_val.as.string = str_create(vm_get_str(vm), data_str, runtime_strlen(data_str));
    }

    if (var_name[0] != '\0') {
        var_assign(vm_get_var(vm), var_name, rec_val);
    }

    return err;
}
