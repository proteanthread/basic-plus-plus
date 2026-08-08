/**
 * @file screen.c
 * @brief SCREEN mode statement handler for BASIC++.
 */
#include "statements/graphics/screen.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_screen_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SCREEN",
        .category = "Graphics & Display",
        .syntax = "SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]]",
        .help_text = "Sets text or visual graphics display mode, screen resolution, and active palette pages.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_screen_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int mode = 0;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            mode = (int)val.as.number;
            if (mode < 0 || mode > 13) {
                err.code = 5;
                err.message = "Illegal Function Call";
                return err;
            }
        }
    }

    (void)mode;
    return err;
}

BppError stmt_title_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_screenmove_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_fullscreen_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_resize_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_icon_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_freeimage_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
BppError stmt_putimage_handler(VMContext *vm, LexerContext *lex) { (void)vm; (void)lex; BppError err; memset(&err, 0, sizeof(err)); return err; }
