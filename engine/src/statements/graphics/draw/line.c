// FILENAME: line.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, input_file.h, input_file.c)
// NEEDS: libengine (lexer.h, lexer.c, line.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the LINE statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/line.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"

void stmt_line_register(void) {
    MicroLibMetadata meta = {
        .name = "LINE",
        .category = "Graphics & Display",
        .syntax = "LINE [[x1, y1]]-(x2, y2) [, [color] [, [B|BF] [, style]]]",
        .help_text = "Draws a straight line or box on the active graphics screen canvas.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
#include "statements/filesystem/file_ops/input_file.h"
#include "platform/platform.h"
#include <string.h>

BppError stmt_line_handler(VMContext *vm, LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    bool is_input = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) ||
                    (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "INPUT", 5) == 0);
    if (is_input) {
        lex_next(lex); // Consume INPUT
        return stmt_line_input_handler(vm, lex);
    }
    return vdev_legacy_stmt_line_handler(vm, lex);
}
