// FILENAME: perform.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (call.h, call.c, perform.h, string.c)
// Provides runtime implementation for the PERFORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/perform.h"
#include "statements/oop/call.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_perform_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PERFORM",
        .category = "Control Flow",
        .syntax = "PERFORM routine_name [(arg1, arg2, ...)]",
        .help_text = "Apple /// Business BASIC alias for CALL. Invokes a subprogram or driver routine with arguments.",
        .error_codes = "Error 2: Syntax Error, Error 35: Subprogram Not Defined"
    };
    microlib_register(&meta);
}

BppError stmt_perform_handler(VMContext *vm, LexerContext *lex) {
    return stmt_call_handler(vm, lex);
}
