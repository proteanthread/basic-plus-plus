// FILENAME: prefix.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (chdir.h, chdir.c, prefix.h, string.c)
// Provides runtime implementation for the PREFIX statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/prefix.h"
#include "statements/filesystem/dir_ops/chdir.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_prefix_desc = {
    .name = "PREFIX",
    .category = "Filesystem",
    .syntax = "PREFIX dir_path$",
    .description = "Apple /// Business BASIC alias for CHDIR. Sets the default SOS directory prefix.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 76: Path Not Found",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_prefix_register(void) {
    lang_desc_register(&g_prefix_desc);
}

BppError stmt_prefix_handler(VMContext *vm, LexerContext *lex) {
    return stmt_chdir_handler(vm, lex);
}
