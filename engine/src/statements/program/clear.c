// FILENAME: clear.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (clear.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CLEAR statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/clear.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "runtime/variables.h"
#include "runtime/arrays.h"

BppError stmt_clear_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));

    VariableContext *vc = vm_get_var(vm);
    if (vc) var_clear_all(vc);

    ArrayContext *ac = vm_get_arr(vm);
    if (ac) arr_clear_all(ac);

    return err;
}

void stmt_clear_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CLEAR",
        .category = "Variables & Memory",
        .syntax = "CLEAR [, [basic_stack_size] [, exec_stack_size]]",
        .help_text = "Frees variable memory, closes open files, and resets sound/graphics channels.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

