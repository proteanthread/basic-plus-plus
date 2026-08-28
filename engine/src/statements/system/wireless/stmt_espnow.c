// FILENAME: stmt_espnow.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libserver
// Implements the ESPNOW statement for fast peer-to-peer radio linking.
//
// ---- Includes ----

#include "statements/system/wireless/stmt_espnow.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "iot_net.h"
#include <string.h>

BppError stmt_espnow_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_init = false;
    bool is_add_peer = false;
    bool is_send = false;
    bool is_recv = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 8 && runtime_strncasecmp(sub.start, "ADD.PEER", 8) == 0) {
                is_add_peer = true;
                lex_next(lex);
            } else if (sub.length == 4 && runtime_strncasecmp(sub.start, "INIT", 4) == 0) {
                is_init = true;
                lex_next(lex);
            } else if (sub.length == 4 && runtime_strncasecmp(sub.start, "SEND", 4) == 0) {
                is_send = true;
                lex_next(lex);
            } else if (sub.length == 4 && runtime_strncasecmp(sub.start, "RECV", 4) == 0) {
                is_recv = true;
                lex_next(lex);
            } else if (sub.length == 3 && runtime_strncasecmp(sub.start, "ADD", 3) == 0) {
                is_add_peer = true;
                lex_next(lex);
                BppToken dot2 = lex_peek(lex);
                if (dot2.type == TOK_PERIOD) {
                    lex_next(lex);
                    BppToken sub2 = lex_peek(lex);
                    if (sub2.type == TOK_IDENT || sub2.type == TOK_KEYWORD) {
                        lex_next(lex);
                    }
                }
            } else {
                lex_next(lex);
            }
        }
    } else {
        is_send = true;
    }

    if (is_init) {
        int ch = 1;
        BppToken peek = lex_peek(lex);
        if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (ch_val.type == VAL_NUMBER) ch = (int)ch_val.as.number;
        }
        iot_espnow_init(ch);
        return err;
    }

    if (is_add_peer) {
        BValue mac_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        const char *mac = (mac_val.type == VAL_STRING) ? str_data(mac_val.as.string) : "";
        int ch = 1;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && ch_val.type == VAL_NUMBER) ch = (int)ch_val.as.number;
        }
        iot_espnow_add_peer(mac, ch, NULL);
        if (mac_val.type == VAL_STRING) str_release(vm_get_str(vm), mac_val.as.string);
        return err;
    }

    if (is_recv) {
        tok = lex_next(lex);
        if (tok.type == TOK_IDENT) {
            char var_name[64];
            size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, nlen);
            var_name[nlen] = '\0';
            char buf[256] = {0};
            char sender_mac[32] = {0};
            int rlen = iot_espnow_recv(buf, sizeof(buf), sender_mac, sizeof(sender_mac));
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), (rlen > 0) ? buf : "", (rlen > 0) ? (size_t)rlen : 0);
            var_assign(vm_get_var(vm), var_name, val);
        }
        return err;
    }

    // Default: ESPNOW.SEND or ESPNOW mac$, data$
    BValue mac_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue data_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (mac_val.type == VAL_STRING) str_release(vm_get_str(vm), mac_val.as.string);
        return err;
    }

    const char *mac = (mac_val.type == VAL_STRING) ? str_data(mac_val.as.string) : "FF:FF:FF:FF:FF:FF";
    if (data_val.type == VAL_STRING) {
        iot_espnow_send(mac, str_data(data_val.as.string), str_len(data_val.as.string));
        str_release(vm_get_str(vm), data_val.as.string);
    }
    if (mac_val.type == VAL_STRING) str_release(vm_get_str(vm), mac_val.as.string);

    return err;
}

void stmt_espnow_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ESPNOW",
        .category = "Wireless & IoT",
        .syntax = "ESPNOW.INIT [channel] | ESPNOW.ADD.PEER mac$ [, channel] | ESPNOW.SEND mac$, data$ | ESPNOW.RECV var$",
        .help_text = "Transmits connectionless low-latency 2.4 GHz packets between ESP32 peers without Wi-Fi router.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
