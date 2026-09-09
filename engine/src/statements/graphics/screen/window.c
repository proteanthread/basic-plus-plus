// FILENAME: window.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, window.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the WINDOW statement in BASIC++.
//
// ---- Includes ----

#include "statements/graphics/screen/window.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_window_desc = {
    .name = "WINDOW",
    .category = "Graphics & Coordinates",
    .syntax = "WINDOW [[SCREEN] (x1, y1)-(x2, y2)]",
    .description = "Defines world coordinate mapping for graphics viewport transformation.",
    .error_summary = "Error 5: Illegal Function Call (coordinates out of bounds)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_window_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        return err;
    }

    // Check for optional SCREEN keyword
    if ((tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) &&
        tok.length == 6 && runtime_strncasecmp(tok.start, "SCREEN", 6) == 0) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_LBRACKET) {
        lex_next(lex); // consume [
        if (lex_peek(lex).type == TOK_LPAREN) {
            lex_next(lex);
            BValue x1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
            BValue y1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
            if (lex_peek(lex).start && lex_peek(lex).start[0] == '-') lex_next(lex);
            if (lex_peek(lex).type == TOK_LPAREN) lex_next(lex);
            BValue x2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
            BValue y2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
            (void)x1; (void)y1; (void)x2; (void)y2;
        } else {
            BValue x1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
            } else if (lex_peek(lex).type == TOK_PERIOD) {
                lex_next(lex);
                if (lex_peek(lex).type == TOK_PERIOD) lex_next(lex);
            }
            BValue x2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
            BValue y1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
            } else if (lex_peek(lex).type == TOK_PERIOD) {
                lex_next(lex);
                if (lex_peek(lex).type == TOK_PERIOD) lex_next(lex);
            }
            BValue y2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
            (void)x1; (void)x2; (void)y1; (void)y2;
        }
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
    } else if (tok.type == TOK_LPAREN) {
        // Parse WINDOW (x1, y1)-(x2, y2)
        lex_next(lex); // consume (
        BValue x1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        BValue y1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
        if (lex_peek(lex).start && lex_peek(lex).start[0] == '-') lex_next(lex);
        if (lex_peek(lex).type == TOK_LPAREN) lex_next(lex);
        BValue x2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        BValue y2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);
        (void)x1; (void)y1; (void)x2; (void)y2;
    } else {
        // Parse WINDOW x1, x2, y1, y2
        BValue x1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        BValue x2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        BValue y1 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        BValue y2 = eval_expression(vm, lex, &err); if (err.code != 0) return err;
        (void)x1; (void)x2; (void)y1; (void)y2;
    }

    return err;
}

void stmt_window_register(void) {
    lang_desc_register(&g_window_desc);
}
