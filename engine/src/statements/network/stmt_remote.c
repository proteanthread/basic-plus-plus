// FILENAME: stmt_remote.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (nil_transport.h, nil_transport.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, stmt_remote.h, string.c)
// Implements REMOTE.EXEC target$, cmd$ statement handler.
//
// ---- Includes ----

#include "statements/network/stmt_remote.h"
#include "runtime/nil_transport.h"
#include "eval/eval.h"
#include "runtime/strings.h"

#include <string.h>

BppError stmt_remote_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 4 && strncasecmp(tok.start, "EXEC", 4) == 0) {
            lex_next(lex);
        }
    }

    BValue target_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    const char *target = (target_val.type == VAL_STRING && target_val.as.string) ? str_data(target_val.as.string) : "COM3";

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 4 && strncasecmp(tok.start, "EXEC", 4) == 0) {
            lex_next(lex);
        }
    }

    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);
        return err;
    }

    const char *cmd = (cmd_val.type == VAL_STRING && cmd_val.as.string) ? str_data(cmd_val.as.string) : "";

    nil_transport_send(vm, target, (const uint8_t *)cmd, strlen(cmd));

    if (cmd_val.type == VAL_STRING && cmd_val.as.string) str_release(vm_get_str(vm), cmd_val.as.string);
    if (target_val.type == VAL_STRING && target_val.as.string) str_release(vm_get_str(vm), target_val.as.string);

    return err;
}

void stmt_remote_register(void) {
    // Registered in VM dispatch
}
