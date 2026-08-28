// FILENAME: mat_write.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (mat_print.h, mat_print.c, mat_write.h)
// Provides runtime implementation for the MAT_WRITE statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/io/mat_write.h"
#include "statements/matrices/io/mat_print.h"
#include "runtime/micro_lib_metadata.h"

void stmt_mat_write_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAT WRITE",
        .category = "Matrix Operations",
        .syntax = "MAT WRITE [#channel,] array_name [;|,]",
        .help_text = "Outputs formatted matrix elements to an open file stream or console (Timesharing Matrix File I/O).",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_mat_write_handler(VMContext *vm, LexerContext *lex) {
    return stmt_mat_print_handler(vm, lex);
}
