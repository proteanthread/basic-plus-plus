// FILENAME: prefix.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (chdir.h, chdir.c, prefix.h, string.c)
// Provides runtime implementation for the PREFIX statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/prefix.h"
#include "statements/filesystem/dir_ops/chdir.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_prefix_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PREFIX",
        .category = "Filesystem",
        .syntax = "PREFIX dir_path$",
        .help_text = "Apple /// Business BASIC alias for CHDIR. Sets the default SOS directory prefix.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 76: Path Not Found"
    };
    microlib_register(&meta);
}

BppError stmt_prefix_handler(VMContext *vm, LexerContext *lex) {
    return stmt_chdir_handler(vm, lex);
}
