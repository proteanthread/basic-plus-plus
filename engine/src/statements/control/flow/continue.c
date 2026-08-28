// FILENAME: continue.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (continue.h, lexer.h, lexer.c, resume.h, resume.c, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the CONTINUE statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/continue.h"
#include "statements/event/resume.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_continue_handler(VMContext *vm, LexerContext *lex) {
    return stmt_resume_handler(vm, lex);
}

void stmt_continue_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CONTINUE",
        .category = "Control Flow",
        .syntax = "CONTINUE",
        .help_text = "ECMA-116 standard statement to continue execution after an exception.",
        .error_codes = "Error 20: RESUME Without Error"
    };
    microlib_register(&meta);
}
