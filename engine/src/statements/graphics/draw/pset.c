// FILENAME: pset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, pset.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PSET statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/pset.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex);

BppError stmt_pset_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_pset_handler(vm, lex);
}

void stmt_pset_register(void) {
    MicroLibMetadata meta = {
        .name = "PSET",
        .category = "Graphics & Drawing",
        .syntax = "PSET (x, y) [, color]",
        .help_text = "Draws a pixel at coordinates (x, y) with specified foreground color.",
        .error_codes = "Error 5: Illegal Function Call (coordinates out of bounds)"
    };
    microlib_register(&meta);
}
