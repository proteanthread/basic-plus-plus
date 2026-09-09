// FILENAME: paint.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, paint.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PAINT statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/paint.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_paint_desc = {
    .name = "PAINT",
    .category = "Graphics & Drawing",
    .syntax = "PAINT (x, y) [, fill_color [, border_color]]",
    .description = "Fills a closed graphics region starting at coordinates (x, y) with color or tile pattern.",
    .error_summary = "Error 5: Illegal Function Call (coordinates out of bounds)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex);

BppError stmt_paint_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_paint_handler(vm, lex);
}

void stmt_paint_register(void) {
    lang_desc_register(&g_paint_desc);
}
