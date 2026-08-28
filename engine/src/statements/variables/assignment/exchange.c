// FILENAME: exchange.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (exchange.h, string.c, swap.h, swap.c)
// Provides runtime implementation for the EXCHANGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/exchange.h"
#include "statements/variables/assignment/swap.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_exchange_register(void) {
    static const MicroLibMetadata meta = {
        .name = "EXCHANGE",
        .category = "Variables & Memory",
        .syntax = "EXCHANGE var1, var2",
        .help_text = "Apple /// Business BASIC alias for SWAP. Exchanges values between two variables or array elements.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_exchange_handler(VMContext *vm, LexerContext *lex) {
    return stmt_swap_handler(vm, lex);
}
