// FILENAME: retry.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, resume.h, resume.c, retry.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the RETRY statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/retry.h"
#include "statements/event/resume.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_retry_handler(VMContext *vm, LexerContext *lex) {
    return stmt_resume_handler(vm, lex);
}

void stmt_retry_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RETRY",
        .category = "Control Flow",
        .syntax = "RETRY",
        .help_text = "ECMA-116 standard statement to re-execute the statement that caused an exception.",
        .error_codes = "Error 20: RESUME Without Error"
    };
    microlib_register(&meta);
}
