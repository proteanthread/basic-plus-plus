// FILENAME: circle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
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
#include "runtime/micro_lib_metadata.h"

void stmt_circle_register(void) {
    MicroLibMetadata meta = {
        .name = "CIRCLE",
        .category = "Graphics & Display",
        .syntax = "CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]",
        .help_text = "Draws a circle, ellipse, or arc on the active graphics screen canvas.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
#include <string.h>

BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex);

BppError stmt_circle_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_circle_handler(vm, lex);
}
