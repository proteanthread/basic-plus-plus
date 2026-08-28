// FILENAME: stmt_peer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, peer.h, peer.c)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, events_net.h, events_net.c, stmt_peer.h)
// Implements the universal PEER statement for connectionless peer-to-peer linking.
//
// ---- Includes ----

#include "statements/network/stmt_peer.h"
#include "runtime/peer.h"
#include "eval/eval.h"
#include "vm/events_net.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_peer_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_init = false;
    bool is_add = false;
    bool is_send = false;
    bool is_recv = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "INIT", 4) == 0) is_init = true;
            if (sub.length == 3 && runtime_strncasecmp(sub.start, "ADD", 3) == 0) is_add = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "SEND", 4) == 0) is_send = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "RECV", 4) == 0) is_recv = true;
            lex_next(lex);

            // Check optional .PEER suffix (e.g. PEER.ADD.PEER)
            if (is_add && lex_peek(lex).type == TOK_PERIOD) {
                lex_next(lex);
                BppToken sub2 = lex_peek(lex);
                if (sub2.length == 4 && runtime_strncasecmp(sub2.start, "PEER", 4) == 0) {
                    lex_next(lex);
                }
            }
        }
    }

    // Check PEER ON / OFF / STOP
    tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
            (tok.length == 3 && runtime_strncasecmp(tok.start, "OFF", 3) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 1, 0);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) ||
            (tok.length == 2 && runtime_strncasecmp(tok.start, "ON", 2) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 1, 1);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_STOP) ||
            (tok.length == 4 && runtime_strncasecmp(tok.start, "STOP", 4) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 1, 2);
            return err;
        }
    }

    if (is_init) {
        int ch = 1;
        tok = lex_peek(lex);
        if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && (ch_val.type == VAL_NUMBER || ch_val.type == VAL_INTEGER)) {
                ch = (int)ch_val.as.number;
            }
        }
        return peer_init(ch);
    }

    if (is_add) {
        BValue mac_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int ch = 1;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && (ch_val.type == VAL_NUMBER || ch_val.type == VAL_INTEGER)) {
                ch = (int)ch_val.as.number;
            }
        }
        const char *mac = (mac_val.type == VAL_STRING) ? str_data(mac_val.as.string) : "";
        BppError add_err = peer_add(mac, ch);
        if (mac_val.type == VAL_STRING) str_release(vm_get_str(vm), mac_val.as.string);
        return add_err;
    }

    if (is_recv) {
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 5;
            err.message = "Syntax error in PEER.RECV: variable name required";
            return err;
        }
        char var_name[64];
        size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : (sizeof(var_name) - 1);
        runtime_memcpy(var_name, tok.start, nlen);
        var_name[nlen] = '\0';

        char payload[PEER_MAX_PAYLOAD + 1];
        size_t out_len = 0;
        char sender[64];
        BppError recv_err = peer_recv(payload, sizeof(payload), &out_len, sender, sizeof(sender));
        if (recv_err.code == 0 && out_len > 0) {
            BValue res;
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), payload, out_len);
            var_assign(vm_get_var(vm), var_name, res);
            str_release(vm_get_str(vm), res.as.string);
        }
        return recv_err;
    }

    // Default: PEER.SEND or PEER peer_id$, data$
    BValue target_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue data_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (target_val.type == VAL_STRING) str_release(vm_get_str(vm), target_val.as.string);
        return err;
    }

    const char *target = (target_val.type == VAL_STRING) ? str_data(target_val.as.string) : "";
    const char *data = (data_val.type == VAL_STRING) ? str_data(data_val.as.string) : "";
    size_t len = (data_val.type == VAL_STRING) ? str_len(data_val.as.string) : 0;

    BppError send_err = peer_send(target, data, len);

    if (target_val.type == VAL_STRING) str_release(vm_get_str(vm), target_val.as.string);
    if (data_val.type == VAL_STRING) str_release(vm_get_str(vm), data_val.as.string);
    return send_err;
}

void stmt_peer_register(void) {
    // Registered in VM dispatch
}
