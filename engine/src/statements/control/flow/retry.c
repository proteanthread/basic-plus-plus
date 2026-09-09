// FILENAME: retry.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, resume.h, resume.c, retry.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the RETRY statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/retry.h"
#include "statements/event/resume.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_retry_desc = {
    .name = "RETRY",
    .category = "Control Flow",
    .syntax = "RETRY",
    .description = "ECMA-116 standard statement to re-execute the statement that caused an exception.",
    .error_summary = "Error 20: RESUME Without Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_retry_handler(VMContext *vm, LexerContext *lex) {
    return stmt_resume_handler(vm, lex);
}

void stmt_retry_register(void) {
    lang_desc_register(&g_retry_desc);
}
