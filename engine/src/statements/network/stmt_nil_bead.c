// FILENAME: stmt_nil_bead.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_net_config.c)
// NEEDS: libcore (nil_bead.h, nil_bead.c, string.h, strings.h, strings.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, stmt_nil_bead.h, string.c)
// Implements NET.UNPACK and NIL.UNPACK statement handlers.
//
// ---- Includes ----

#include "statements/network/stmt_nil_bead.h"
#include "runtime/nil_bead.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/variables.h"

#include <string.h>

BppError stmt_nil_unpack_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 6 && strncasecmp(tok.start, "UNPACK", 6) == 0) {
            lex_next(lex);
        }
    }

    BValue pkt_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (pkt_val.type != VAL_STRING || !pkt_val.as.string) {
        err.code = 13;
        err.message = "NET.UNPACK expects binary packet string";
        if (pkt_val.type == VAL_STRING && pkt_val.as.string) str_release(vm_get_str(vm), pkt_val.as.string);
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BppToken var_tok = lex_next(lex);
    if (var_tok.type != TOK_IDENT && var_tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Expected destination variable in NET.UNPACK";
        str_release(vm_get_str(vm), pkt_val.as.string);
        return err;
    }

    char var_name[64];
    size_t nlen = var_tok.length < sizeof(var_name) - 1 ? var_tok.length : sizeof(var_name) - 1;
    memcpy(var_name, var_tok.start, nlen);
    var_name[nlen] = '\0';

    const uint8_t *data = (const uint8_t *)str_data(pkt_val.as.string);
    size_t len = str_len(pkt_val.as.string);

    BValue unpacked_val;
    if (nil_bead_deserialize_value(vm, data, len, &unpacked_val)) {
        var_assign(vm_get_var(vm), var_name, unpacked_val);
    }

    str_release(vm_get_str(vm), pkt_val.as.string);
    return err;
}

void stmt_nil_unpack_register(void) {
    // Registered in VM dispatch
}
