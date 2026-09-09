// FILENAME: vdim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (dim.h, dim.c, vdim.h)
// Provides runtime implementation for the VDIM statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/binary_ops/vdim.h"
#include "statements/variables/declaration/dim.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_dim_desc = {
    .name = "DIM #",
    .category = "File System & Virtual Memory",
    .syntax = "DIM #channel, array_name(bounds)",
    .description = "Allocates a DEC BASIC-PLUS virtual array mapped to an open random or sequential file channel.",
    .error_summary = "Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 10: Array Already Dimensioned, Error 52: Bad File Number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_vdim_register(void) {
    lang_desc_register(&g_dim_desc);
}

BppError stmt_vdim_handler(VMContext *vm, LexerContext *lex) {
    return stmt_dim_handler(vm, lex);
}
