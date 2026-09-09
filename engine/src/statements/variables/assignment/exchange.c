// FILENAME: exchange.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (exchange.h, string.c, swap.h, swap.c)
// Provides runtime implementation for the EXCHANGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/exchange.h"
#include "statements/variables/assignment/swap.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_exchange_desc = {
    .name = "EXCHANGE",
    .category = "Variables & Memory",
    .syntax = "EXCHANGE var1, var2",
    .description = "Apple /// Business BASIC alias for SWAP. Exchanges values between two variables or array elements.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_exchange_register(void) {
    lang_desc_register(&g_exchange_desc);
}

BppError stmt_exchange_handler(VMContext *vm, LexerContext *lex) {
    return stmt_swap_handler(vm, lex);
}
