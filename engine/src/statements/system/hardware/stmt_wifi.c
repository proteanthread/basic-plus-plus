// FILENAME: stmt_wifi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements WIFI.CONNECT and WIFI.DISCONNECT statements for wireless networking.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "iot_net.h"
#include <string.h>

BppError stmt_wifi_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_disconnect = false;
    bool is_ap = false;
    bool is_scan = false;
    bool is_sniff = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 10 && runtime_strncasecmp(sub.start, "DISCONNECT", 10) == 0) is_disconnect = true;
            if (sub.length == 2 && runtime_strncasecmp(sub.start, "AP", 2) == 0) is_ap = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "SCAN", 4) == 0) is_scan = true;
            if (sub.length == 5 && runtime_strncasecmp(sub.start, "SNIFF", 5) == 0) is_sniff = true;
            lex_next(lex);
        }
    }

    if (is_disconnect) {
        iot_wifi_disconnect();
        return err;
    }

    if (is_scan) {
        iot_wifi_scan();
        return err;
    }

    if (is_sniff) {
        BppToken peek = lex_peek(lex);
        bool sniff_on = true;
        if (peek.type == TOK_IDENT || peek.type == TOK_KEYWORD) {
            if ((peek.type == TOK_KEYWORD && peek.as.keyword == KW_OFF) ||
                (peek.length == 3 && runtime_strncasecmp(peek.start, "OFF", 3) == 0)) {
                sniff_on = false;
            }
            lex_next(lex);
        }
        int ch = 1;
        peek = lex_peek(lex);
        if (peek.type == TOK_COMMA) {
            lex_next(lex);
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && ch_val.type == VAL_NUMBER) ch = (int)ch_val.as.number;
        }
        if (sniff_on) {
            iot_wifi_sniff_start(ch);
        } else {
            iot_wifi_sniff_stop();
        }
        return err;
    }

    BValue ssid_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue pass_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (ssid_val.type == VAL_STRING) str_release(vm_get_str(vm), ssid_val.as.string);
        return err;
    }

    const char *ssid = (ssid_val.type == VAL_STRING) ? str_data(ssid_val.as.string) : "";
    const char *pass = (pass_val.type == VAL_STRING) ? str_data(pass_val.as.string) : "";

    if (is_ap) {
        int ch = 1;
        int max_cl = 4;
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && ch_val.type == VAL_NUMBER) ch = (int)ch_val.as.number;
            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                BValue cl_val = eval_expression(vm, lex, &err);
                if (err.code == 0 && cl_val.type == VAL_NUMBER) max_cl = (int)cl_val.as.number;
            }
        }
        iot_wifi_ap_start(ssid, pass, ch, max_cl);
    } else {
        iot_wifi_connect(ssid, pass);
    }

    if (ssid_val.type == VAL_STRING) str_release(vm_get_str(vm), ssid_val.as.string);
    if (pass_val.type == VAL_STRING) str_release(vm_get_str(vm), pass_val.as.string);
    return err;
}

void stmt_wifi_register(void) {
    static const MicroLibMetadata meta = {
        .name = "WIFI",
        .category = "Wireless & IoT",
        .syntax = "WIFI.CONNECT ssid$, pass$ | WIFI.DISCONNECT | WIFI.AP ssid$, pass$ [, ch, max] | WIFI.SCAN | WIFI.SNIFF ON|OFF [, ch]",
        .help_text = "Controls wireless 802.11 b/g/n station client, SoftAP hosting, scanning, and promiscuous packet sniffing.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
