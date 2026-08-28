// FILENAME: paint.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, paint.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PAINT statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/paint.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include <string.h>

BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex);

BppError stmt_paint_handler(VMContext *vm, LexerContext *lex) {
    return vdev_legacy_stmt_paint_handler(vm, lex);
}

void stmt_paint_register(void) {
    MicroLibMetadata meta = {
        .name = "PAINT",
        .category = "Graphics & Drawing",
        .syntax = "PAINT (x, y) [, fill_color [, border_color]]",
        .help_text = "Fills a closed graphics region starting at coordinates (x, y) with color or tile pattern.",
        .error_codes = "Error 5: Illegal Function Call (coordinates out of bounds)"
    };
    microlib_register(&meta);
}
