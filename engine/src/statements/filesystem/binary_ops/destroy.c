// FILENAME: destroy.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (destroy.h, kill.h, kill.c)
// Provides runtime implementation for the DESTROY statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/destroy.h"
#include "statements/filesystem/dir_ops/kill.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_destroy_desc = {
    .name = "DESTROY",
    .category = "Filesystem",
    .syntax = "DESTROY filename$",
    .description = "Apple /// Business BASIC alias for KILL. Deletes a file from the disk.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_destroy_register(void) {
    lang_desc_register(&g_destroy_desc);
}

BppError stmt_destroy_handler(VMContext *vm, LexerContext *lex) {
    return stmt_kill_handler(vm, lex);
}
