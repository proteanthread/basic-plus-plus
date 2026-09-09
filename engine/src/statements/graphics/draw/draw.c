// FILENAME: draw.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bgi.h, bgi.c, draw.h, eval.h, eval.c, lexer.h, lexer.c)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the DRAW statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/draw/draw.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "runtime/strings.h"
#include "device/bgi.h"

static const LangDesc g_draw_desc = {
    .name = "DRAW",
    .category = "Graphics & Drawing",
    .syntax = "DRAW command_str$",
    .description = "Executes graphics macro commands to draw shapes using vector movement language.",
    .error_summary = "Error 5: Illegal Function Call (invalid DRAW command string)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_draw_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        err.code = 2; err.message = "Expected drawing commands in DRAW statement";
        return err;
    }

    BValue str_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (str_val.type != VAL_STRING) {
        err.code = 13; err.message = "DRAW expects string expression";
        return err;
    }

    BGI_Context *ctx = BGI_get_global_context();
    if (ctx && ctx->initialized && ctx->framebuffer) {
        const char *cmd_str = str_data(str_val.as.string);
        if (!cmd_str) cmd_str = "";
        BGI_draw(ctx, cmd_str);
        BGI_present(ctx);
    }

    str_release(vm_get_str(vm), str_val.as.string);
    return err;
}

void stmt_draw_register(void) {
    lang_desc_register(&g_draw_desc);
}
