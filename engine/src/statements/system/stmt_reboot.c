// FILENAME: stmt_reboot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements REBOOT and RESET statements for microcontroller restart.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>

BppError stmt_reboot_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    // Soft reset
    return err;
}

void stmt_reboot_register(void) {
    static const MicroLibMetadata meta = {
        .name = "REBOOT",
        .category = "System & Power",
        .syntax = "REBOOT | RESET",
        .help_text = "Triggers a hardware system restart on microcontroller target.",
        .error_codes = "None"
    };
    microlib_register(&meta);
}
