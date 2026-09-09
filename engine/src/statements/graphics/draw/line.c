// FILENAME: line.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
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
#include "runtime/language_descriptor.h"


#include "statements/filesystem/file_ops/input_file.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_line_desc = {
    .name = "LINE",
    .category = "Graphics & Display",
    .syntax = "LINE [[x1, y1]]-(x2, y2) [, [color] [, [B|BF] [, style]]]",
    .description = "Draws a straight line or box on the active graphics screen canvas.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_line_register(void) {
    lang_desc_register(&g_line_desc);
}
BppError stmt_line_handler(VMContext *vm, LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    bool is_input = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_INPUT) ||
                    (tok.type == TOK_IDENT && tok.length == 5 && platform_strncasecmp(tok.start, "INPUT", 5) == 0);
    if (is_input) {
        lex_next(lex); // Consume INPUT
        return stmt_line_input_handler(vm, lex);
    }
    if (tok.type == TOK_EOL || tok.type == TOK_EOF ||
        (tok.type == TOK_KEYWORD && tok.as.keyword == KW_IF) ||
        (tok.type == TOK_KEYWORD && tok.as.keyword == KW_UNLESS) ||
        (tok.type == TOK_KEYWORD && tok.as.keyword == KW_FOR)) {
        // JOSS LINE statement (outputs a newline)
        VDevContext *vdev = vm_get_vdev(vm);
        if (vdev) vdev_puts(vdev, "\n");
        BppError err = {0};
        return err;
    }
    return vdev_legacy_stmt_line_handler(vm, lex);
}
