// FILENAME: return.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (return.h, string.c, vm.h)
// Provides runtime implementation for the RETURN statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/return.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_return_desc = {
    .name = "RETURN",
    .category = "Control Flow",
    .syntax = "RETURN [line_num]",
    .description = "Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line.",
    .error_summary = "Error 3: RETURN without GOSUB (call stack empty)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_return_register(void) {
    lang_desc_register(&g_return_desc);
}

BppError stmt_return_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    BppLineNumber resume_line = 0.0;
    const char *resume_pos = NULL;

    if (!vm_gosub_pop(vm, &resume_line, &resume_pos)) {
        err.code = 3; // RETURN without GOSUB
        err.message = "RETURN without GOSUB";
        return err;
    }

    vm_jump(vm, resume_line, resume_pos);
    vm_clear_event_handlers(vm);
    return err;
}
