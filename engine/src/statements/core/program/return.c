// FILENAME: return.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (return.h, string.c, vm.h)
// Provides runtime implementation for the RETURN statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/return.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include <string.h>

void stmt_return_register(void) {
    MicroLibMetadata meta = {
        .name = "RETURN",
        .category = "Control Flow",
        .syntax = "RETURN [line_num]",
        .help_text = "Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line.",
        .error_codes = "Error 3: RETURN without GOSUB (call stack empty)"
    };
    microlib_register(&meta);
}

BppError stmt_return_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
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
