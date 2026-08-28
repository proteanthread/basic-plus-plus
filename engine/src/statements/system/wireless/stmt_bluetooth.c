// FILENAME: stmt_bluetooth.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_bluetooth.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements BT and BLE statements for Bluetooth Classic and BLE operations.
//
// ---- Includes ----

#include "statements/system/wireless/stmt_bluetooth.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "iot_net.h"
#include <string.h>

BppError stmt_bt_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_start = false;
    bool is_connect = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "START", 5) == 0) is_start = true;
        else if (tok.length == 7 && runtime_strncasecmp(tok.start, "CONNECT", 7) == 0) is_connect = true;
        if (is_start || is_connect) lex_next(lex);
    } else {
        is_start = true;
    }

    BValue arg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    const char *arg_str = (arg_val.type == VAL_STRING) ? str_data(arg_val.as.string) : "ESP32-BT";

    if (is_connect) {
        iot_bt_spp_connect(arg_str);
    } else {
        iot_bt_spp_start(arg_str);
    }

    if (arg_val.type == VAL_STRING) str_release(vm_get_str(vm), arg_val.as.string);
    return err;
}

BppError stmt_ble_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_adv = false;
    bool is_adv_stop = false;
    bool is_scan = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        if (tok.length == 8 && runtime_strncasecmp(tok.start, "ADV.STOP", 8) == 0) {
            is_adv_stop = true;
            lex_next(lex);
        } else if (tok.length == 9 && runtime_strncasecmp(tok.start, "ADV.START", 9) == 0) {
            is_adv = true;
            lex_next(lex);
        } else if (tok.length == 3 && runtime_strncasecmp(tok.start, "ADV", 3) == 0) {
            is_adv = true;
            lex_next(lex);
            BppToken dot2 = lex_peek(lex);
            if (dot2.type == TOK_PERIOD) {
                lex_next(lex);
                BppToken sub2 = lex_peek(lex);
                if (sub2.type == TOK_IDENT || sub2.type == TOK_KEYWORD) {
                    if (sub2.length == 4 && runtime_strncasecmp(sub2.start, "STOP", 4) == 0) {
                        is_adv_stop = true;
                    }
                    lex_next(lex);
                }
            }
        } else if (tok.length == 4 && runtime_strncasecmp(tok.start, "SCAN", 4) == 0) {
            is_scan = true;
            lex_next(lex);
        }
    }

    if (is_adv_stop) {
        iot_ble_adv_stop();
        return err;
    }

    if (is_scan) {
        int duration_ms = 5000;
        BppToken peek = lex_peek(lex);
        if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
            BValue d_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && d_val.type == VAL_NUMBER) duration_ms = (int)d_val.as.number;
        }
        iot_ble_scan(duration_ms);
        return err;
    }

    if (is_adv) {
        BValue name_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);

        BValue uuid_val = eval_expression(vm, lex, &err);
        const char *name = (name_val.type == VAL_STRING) ? str_data(name_val.as.string) : "BLE-Beacon";
        const char *uuid = (uuid_val.type == VAL_STRING) ? str_data(uuid_val.as.string) : "0000";

        iot_ble_adv_start(name, uuid);

        if (uuid_val.type == VAL_STRING) str_release(vm_get_str(vm), uuid_val.as.string);
        if (name_val.type == VAL_STRING) str_release(vm_get_str(vm), name_val.as.string);
        return err;
    }

    return err;
}

void stmt_bt_register(void) {
    static const MicroLibMetadata meta = {
        .name = "BT",
        .category = "Wireless & IoT",
        .syntax = "BT.START name$ | BT.CONNECT mac_or_name$",
        .help_text = "Controls Bluetooth Classic Serial Port Profile (SPP) virtual COM link.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

void stmt_ble_register(void) {
    static const MicroLibMetadata meta = {
        .name = "BLE",
        .category = "Wireless & IoT",
        .syntax = "BLE.ADV.START name$, uuid$ | BLE.ADV.STOP | BLE.SCAN [duration_ms]",
        .help_text = "Controls Bluetooth Low Energy advertising beacons and GATT scanning.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
