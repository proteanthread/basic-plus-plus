// FILENAME: stmt_sock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (sock_engine.h, sock_engine.c, strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, stmt_sock.h)
// Implements BSD SOCK statement handlers.
//
// ---- Includes ----

#include "statements/network/stmt_sock.h"
#include "runtime/sock_engine.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_sock_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_bind = false;
    bool is_listen = false;
    bool is_send = false;
    bool is_close = false;
    bool is_setopt = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "BIND", 4) == 0) is_bind = true;
            if (sub.length == 6 && runtime_strncasecmp(sub.start, "LISTEN", 6) == 0) is_listen = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "SEND", 4) == 0) is_send = true;
            if (sub.length == 5 && runtime_strncasecmp(sub.start, "CLOSE", 5) == 0) is_close = true;
            if (sub.length == 10 && runtime_strncasecmp(sub.start, "SETSOCKOPT", 10) == 0) is_setopt = true;
            lex_next(lex);
        }
    }

    BValue h_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int handle = (h_val.type == VAL_NUMBER || h_val.type == VAL_INTEGER) ? (int)h_val.as.number : 0;

    if (is_close) {
        return sock_close(handle);
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    if (is_bind) {
        BValue p_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int port = (p_val.type == VAL_NUMBER || p_val.type == VAL_INTEGER) ? (int)p_val.as.number : 80;
        return sock_bind(handle, port);
    }

    if (is_listen) {
        int backlog = 5;
        tok = lex_peek(lex);
        if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
            BValue bl_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && (bl_val.type == VAL_NUMBER || bl_val.type == VAL_INTEGER)) backlog = (int)bl_val.as.number;
        }
        return sock_listen(handle, backlog);
    }

    if (is_send) {
        BValue d_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        const char *data = (d_val.type == VAL_STRING) ? str_data(d_val.as.string) : "";
        size_t len = (d_val.type == VAL_STRING) ? str_len(d_val.as.string) : 0;
        BppError send_err = sock_send(handle, data, len);
        if (d_val.type == VAL_STRING) str_release(vm_get_str(vm), d_val.as.string);
        return send_err;
    }

    if (is_setopt) {
        BValue opt_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);
        BValue val_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            if (opt_val.type == VAL_STRING) str_release(vm_get_str(vm), opt_val.as.string);
            return err;
        }
        const char *opt = (opt_val.type == VAL_STRING) ? str_data(opt_val.as.string) : "";
        int v = (val_val.type == VAL_NUMBER) ? (int)val_val.as.number : 0;
        BppError set_err = sock_setsockopt(handle, opt, v);
        if (opt_val.type == VAL_STRING) str_release(vm_get_str(vm), opt_val.as.string);
        return set_err;
    }

    err.code = 5;
    err.message = "Unknown SOCK sub-command";
    return err;
}

void stmt_sock_register(void) {
    // Registered in VM dispatch
}
