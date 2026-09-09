// FILENAME: stmt_jit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore, libengine, libkernel
// Implements the JIT statement for dynamic runtime JIT/AOT mode configuration.
//
// ---- Includes ----

#include "statements/program/stmt_jit.h"
#include "vm/jit.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_jit_desc = {
    .name = "JIT",
    .category = "Program Mgmt & Editing",
    .syntax = "JIT [ON | OFF | AUTO | BYTECODE | NATIVE | FAST | SAFE]",
    .description = "Configures the Just-In-Time and Ahead-Of-Time compilation tier and execution mode.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_jit_register(void) {
    lang_desc_register(&g_jit_desc);
}

BppError stmt_jit_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_ON) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_AUTO);
            return err;
        } else if (tok.as.keyword == KW_OFF) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_OFF);
            return err;
        } else if (tok.as.keyword == KW_AUTO) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_AUTO);
            return err;
        }
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (runtime_strncasecmp(tok.start, "OFF", tok.length) == 0) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_OFF);
            return err;
        } else if (runtime_strncasecmp(tok.start, "ON", tok.length) == 0) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_AUTO);
            return err;
        } else if (runtime_strncasecmp(tok.start, "AUTO", tok.length) == 0) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_AUTO);
            return err;
        } else if (runtime_strncasecmp(tok.start, "BYTECODE", tok.length) == 0) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_BYTECODE);
            return err;
        } else if (runtime_strncasecmp(tok.start, "NATIVE", tok.length) == 0) {
            lex_next(lex);
            jit_set_mode(vm, JIT_MODE_NATIVE);
            return err;
        } else if (runtime_strncasecmp(tok.start, "FAST", tok.length) == 0) {
            lex_next(lex);
            jit_set_fast_mode(vm, true);
            return err;
        } else if (runtime_strncasecmp(tok.start, "SAFE", tok.length) == 0) {
            lex_next(lex);
            jit_set_fast_mode(vm, false);
            return err;
        }
    }

    // Bare JIT statement defaults to auto
    jit_set_mode(vm, JIT_MODE_AUTO);
    return err;
}
