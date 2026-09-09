// FILENAME: pset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, pset.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PSET statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/pset.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_pset_desc = {
    .name = "PSET",
    .category = "Graphics & Drawing",
    .syntax = "PSET (x, y) [, color]",
    .description = "Draws a pixel at coordinates (x, y) with specified foreground color.",
    .error_summary = "Error 5: Illegal Function Call (coordinates out of bounds)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex);

BppError stmt_pset_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_pset_handler(vm, lex);
}

void stmt_pset_register(void) {
    lang_desc_register(&g_pset_desc);
}
