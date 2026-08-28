// FILENAME: cont.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cont.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CONT statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/cont.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_cont_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    vm_set_single_step(vm, false);
    return err;
}

void stmt_cont_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CONT",
        .category = "Program Mgmt & Editing",
        .syntax = "CONT",
        .help_text = "Resumes program execution after a break or STOP statement.",
        .error_codes = "Error 2: Syntax Error, Error 17: Can't Continue"
    };
    microlib_register(&meta);
}

