// FILENAME: preset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, preset.h, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PRESET statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/preset.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_preset_desc = {
    .name = "PRESET",
    .category = "Graphics & Drawing",
    .syntax = "PRESET (x, y) [, color]",
    .description = "Draws a pixel at coordinates (x, y), defaulting to background color.",
    .error_summary = "Error 5: Illegal Function Call (coordinates out of bounds)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex);

BppError stmt_preset_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_preset_handler(vm, lex);
}

void stmt_preset_register(void) {
    lang_desc_register(&g_preset_desc);
}
