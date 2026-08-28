// FILENAME: vdim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (dim.h, dim.c, vdim.h)
// Provides runtime implementation for the VDIM statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/vdim.h"
#include "statements/variables/declaration/dim.h"
#include "runtime/micro_lib_metadata.h"

void stmt_vdim_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DIM #",
        .category = "File System & Virtual Memory",
        .syntax = "DIM #channel, array_name(bounds)",
        .help_text = "Allocates a DEC BASIC-PLUS virtual array mapped to an open random or sequential file channel.",
        .error_codes = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 10: Array Already Dimensioned, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_vdim_handler(VMContext *vm, LexerContext *lex) {
    return stmt_dim_handler(vm, lex);
}
