// FILENAME: destroy.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (destroy.h, kill.h, kill.c)
// Provides runtime implementation for the DESTROY statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/destroy.h"
#include "statements/filesystem/dir_ops/kill.h"
#include "runtime/micro_lib_metadata.h"

void stmt_destroy_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DESTROY",
        .category = "Filesystem",
        .syntax = "DESTROY filename$",
        .help_text = "Apple /// Business BASIC alias for KILL. Deletes a file from the disk.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

BppError stmt_destroy_handler(VMContext *vm, LexerContext *lex) {
    return stmt_kill_handler(vm, lex);
}
