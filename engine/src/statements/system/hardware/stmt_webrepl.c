// FILENAME: stmt_webrepl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements WEBREPL.START and WEBREPL.STOP statements for over-the-air programming.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "iot_net.h"
#include "runtime/string/memops.h"

static const LangDesc g_webrepl_desc = {
    .name = "WEBREPL",
    .category = "Wireless & IoT",
    .syntax = "WEBREPL.START [port] | WEBREPL.STOP",
    .description = "Starts wireless WebSocket WebREPL server for remote terminal interaction.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_webrepl_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_stop = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (tok_is_keyword(sub, KW_STOP, "STOP")) {
            is_stop = true;
            lex_next(lex);
        } else if ((sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) && sub.length == 5 && runtime_strncasecmp(sub.start, "START", 5) == 0) {
            is_stop = false;
            lex_next(lex);
        } else if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            lex_next(lex);
        }
    }

    int port = 8266;
    if (is_stop) {
        iot_webrepl_stop();
    } else {
        BppToken peek = lex_peek(lex);
        if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
            BValue port_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (port_val.type != VAL_STRING && port_val.as.number > 0) {
                port = (int)port_val.as.number;
            }
        }
        iot_webrepl_start(port);
    }
    return err;
}

void stmt_webrepl_register(void) {
    lang_desc_register(&g_webrepl_desc);
}
