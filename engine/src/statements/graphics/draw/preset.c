// FILENAME: preset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, preset.h, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PRESET statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/preset.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <string.h>

BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex);

BppError stmt_preset_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_preset_handler(vm, lex);
}

void stmt_preset_register(void) {
    MicroLibMetadata meta = {
        .name = "PRESET",
        .category = "Graphics & Drawing",
        .syntax = "PRESET (x, y) [, color]",
        .help_text = "Draws a pixel at coordinates (x, y), defaulting to background color.",
        .error_codes = "Error 5: Illegal Function Call (coordinates out of bounds)"
    };
    microlib_register(&meta);
}
