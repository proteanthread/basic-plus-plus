// FILENAME: new.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
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
#include "runtime/arrays.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_new_desc = {
    .name = "NEW",
    .category = "Program Mgmt & Editing",
    .syntax = "NEW",
    .description = "Clears the current program from memory and resets all variables.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_new_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;
    if (!vm) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    mem_program_clear(vm_get_mem(vm));
    var_clear_all(vm_get_var(vm));
    arr_clear_all(vm_get_arr(vm));
    vm_reset_for_run(vm);
    vm_clear_error(vm);
    vm_set_running(vm, true);
    vm_set_current_line(vm, 0.0);
    return err;
}

void stmt_new_register(void) {
    lang_desc_register(&g_new_desc);
}

