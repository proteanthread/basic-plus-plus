// FILENAME: circle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (circle.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CIRCLE statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/circle.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/language_descriptor.h"


#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_circle_desc = {
    .name = "CIRCLE",
    .category = "Graphics & Display",
    .syntax = "CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]",
    .description = "Draws a circle, ellipse, or arc on the active graphics screen canvas.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_circle_register(void) {
    lang_desc_register(&g_circle_desc);
}
BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex);

BppError stmt_circle_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_circle_handler(vm, lex);
}
