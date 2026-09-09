// FILENAME: repeat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (repeat.h, string.c)
// Provides runtime implementation for the REPEAT statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/repeat.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_repeat_desc = {
    .name = "REPEAT",
    .category = "Looping / Control Flow",
    .syntax = "REPEAT",
    .description = "Initiates a BASIC09 post-test REPEAT...UNTIL loop block.",
    .error_summary = "Error 2: Syntax Error, Error 14: Loop Stack Overflow",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_repeat_register(void) {
    lang_desc_register(&g_repeat_desc);
}

BppError stmt_repeat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm_do_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
        err.code = 14;
        err.message = "Out of memory / Loop stack overflow in REPEAT";
        return err;
    }

    return err;
}
