// FILENAME: mat_write.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (mat_print.h, mat_print.c, mat_write.h)
// Provides runtime implementation for the MAT_WRITE statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/io/mat_write.h"
#include "statements/matrices/io/mat_print.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_mat_write_desc = {
    .name = "MAT WRITE",
    .category = "Matrix Operations",
    .syntax = "MAT WRITE [#channel,] array_name [;|,]",
    .description = "Outputs formatted matrix elements to an open file stream or console (Timesharing Matrix File I/O).",
    .error_summary = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_mat_write_register(void) {
    lang_desc_register(&g_mat_write_desc);
}

BppError stmt_mat_write_handler(VMContext *vm, LexerContext *lex) {
    return stmt_mat_print_handler(vm, lex);
}
