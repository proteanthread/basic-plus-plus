// FILENAME: new.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, new.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the NEW statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/new.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_new_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    if (!vm) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    mem_program_clear(vm_get_mem(vm));
    var_clear_all(vm_get_var(vm));
    vm_reset_for_run(vm);
    vm_clear_error(vm);
    vm_set_running(vm, true);
    vm_set_current_line(vm, 0.0);
    return err;
}

void stmt_new_register(void) {
    static const MicroLibMetadata meta = {
        .name = "NEW",
        .category = "Program Mgmt & Editing",
        .syntax = "NEW",
        .help_text = "Clears the current program from memory and resets all variables.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

