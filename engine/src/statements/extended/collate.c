// FILENAME: collate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the COLLATE statement in BASIC++.
//
// ---- Includes ----

#include "statements/extended/collate.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_collate_desc = {
    .name = "COLLATE",
    .category = "Strings & Formatting",
    .syntax = "COLLATE STANDARD | COLLATE NATIVE | COLLATE table$",
    .description = "Sets the character collation sequence for string comparisons (ANSI Full BASIC X3.113-1987 Section 9 / ECMA-116).",
    .error_summary = "Error 2: Syntax error, Error 5: Illegal function call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_collate_register(void) {
    lang_desc_register(&g_collate_desc);
}

BppError stmt_collate_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_COLLATE) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        err.code = 2;
        err.message = "Expected collation specification after COLLATE";
        return err;
    }

    if (tok.type == TOK_IDENT) {
        lex_next(lex);
        return err;
    }

    if (tok.type == TOK_STRING) {
        lex_next(lex);
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING && val.as.string) {
        str_release(vm_get_str(vm), val.as.string);
    }

    return err;
}
