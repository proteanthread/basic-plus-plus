// FILENAME: stmt_yield.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libplatform (platform.h)
// Provides runtime implementation for the YIELD cooperative multitasking statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/stmt_yield.h"
#include "platform/platform.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"

#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_yield_desc = {
    .name = "YIELD",
    .category = "Control Flow",
    .syntax = "YIELD",
    .description = "Yields the active thread execution timeslice to background tasks and OS event pump.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_yield_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    platform_sleep_ms(0); // Yield thread timeslice to OS scheduler
    return err;
}

void stmt_yield_register(void) {
    lang_desc_register(&g_yield_desc);
}
