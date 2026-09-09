// FILENAME: stmt_gopher.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gopher.h, gopher.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_gopher.h, string.c)
// NEEDS: libengine (vm.h)
// Implements GOPHER statement for hosting RFC 1436 Gopher menus and servers.
//
// ---- Includes ----

#include "statements/network/stmt_gopher.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/gopher.h"
#include "runtime/string/memops.h"

static const LangDesc g_gopher_desc = {
    .name = "GOPHER",
    .category = "Network & Cloud",
    .syntax = "GOPHER.SERVE [port] [, root_dir$] | GOPHER.GET$(url$)",
    .description = "Hosts RFC 1436 Gopher directory menus or fetches Gopher items.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

BppError stmt_gopher_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            lex_next(lex);
        }
    }

    int port = 70;
    BppToken peek = lex_peek(lex);
    if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
        BValue port_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (port_val.type == VAL_NUMBER) port = (int)port_val.as.number;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    const char *root_dir = ".";
    peek = lex_peek(lex);
    if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
        BValue root_val = eval_expression(vm, lex, &err);
        if (err.code == 0 && root_val.type == VAL_STRING) {
            root_dir = str_data(root_val.as.string);
            net_gopher_serve(port, root_dir);
            str_release(vm_get_str(vm), root_val.as.string);
            return err;
        }
    }

    net_gopher_serve(port, root_dir);
    return err;
}

void stmt_gopher_register(void) {
    lang_desc_register(&g_gopher_desc);
}
