// FILENAME: extend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (extend.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the EXTEND statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/external/extend.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_extend_desc = {
    .name = "EXTEND",
    .category = "Compiler Directives",
    .syntax = "EXTEND",
    .description = "Enables DEC BASIC-PLUS extended syntax (multi-character identifiers and continuations).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_noextend_desc = {
    .name = "NOEXTEND",
    .category = "Compiler Directives",
    .syntax = "NO EXTEND | NOEXTEND",
    .description = "Disables extended syntax restrictions (classic Minimal BASIC variable rules).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_extend_register(void) {
    lang_desc_register(&g_extend_desc);

    lang_desc_register(&g_noextend_desc);
}

BppError stmt_extend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_noextend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}
