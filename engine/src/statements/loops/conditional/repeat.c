// FILENAME: repeat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (repeat.h, string.c)
// Provides runtime implementation for the REPEAT statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/repeat.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_repeat_register(void) {
    static const MicroLibMetadata meta = {
        .name = "REPEAT",
        .category = "Looping / Control Flow",
        .syntax = "REPEAT",
        .help_text = "Initiates a BASIC09 post-test REPEAT...UNTIL loop block.",
        .error_codes = "Error 2: Syntax Error, Error 14: Loop Stack Overflow"
    };
    microlib_register(&meta);
}

BppError stmt_repeat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm_do_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 14;
        err.message = "Out of memory / Loop stack overflow in REPEAT";
        return err;
    }

    return err;
}
