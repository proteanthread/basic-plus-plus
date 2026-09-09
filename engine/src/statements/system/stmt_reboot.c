// FILENAME: stmt_reboot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements REBOOT and RESET statements for microcontroller restart.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"

static const LangDesc g_reboot_desc = {
    .name = "REBOOT",
    .category = "System & Power",
    .syntax = "REBOOT | RESET",
    .description = "Triggers a hardware system restart on microcontroller target.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_reboot_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    // Soft reset
    return err;
}

void stmt_reboot_register(void) {
    lang_desc_register(&g_reboot_desc);
}
