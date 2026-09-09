// FILENAME: perform.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (call.h, call.c, perform.h, string.c)
// Provides runtime implementation for the PERFORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/perform.h"
#include "statements/oop/call.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_perform_desc = {
    .name = "PERFORM",
    .category = "Control Flow",
    .syntax = "PERFORM routine_name [(arg1, arg2, ...)]",
    .description = "Apple /// Business BASIC alias for CALL. Invokes a subprogram or driver routine with arguments.",
    .error_summary = "Error 2: Syntax Error, Error 35: Subprogram Not Defined",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_perform_register(void) {
    lang_desc_register(&g_perform_desc);
}

BppError stmt_perform_handler(VMContext *vm, LexerContext *lex) {
    return stmt_call_handler(vm, lex);
}
