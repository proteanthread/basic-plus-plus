// FILENAME: stmt_mqtt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements MQTT.PUBLISH and MQTT.SUBSCRIBE statements for IoT cloud messaging.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "iot_net.h"
#include <string.h>

BppError stmt_mqtt_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_sub = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT) {
            if (runtime_strncasecmp(sub.start, "SUBSCRIBE", sub.length) == 0) is_sub = true;
            lex_next(lex);
        }
    }

    BValue topic_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (is_sub) {
        const char *topic = (topic_val.type == VAL_STRING) ? str_data(topic_val.as.string) : "";
        iot_mqtt_subscribe(topic);
        if (topic_val.type == VAL_STRING) str_release(vm_get_str(vm), topic_val.as.string);
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue payload_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        if (topic_val.type == VAL_STRING) str_release(vm_get_str(vm), topic_val.as.string);
        return err;
    }

    const char *topic = (topic_val.type == VAL_STRING) ? str_data(topic_val.as.string) : "";
    const char *payload = (payload_val.type == VAL_STRING) ? str_data(payload_val.as.string) : "";
    iot_mqtt_publish(topic, payload);

    if (topic_val.type == VAL_STRING) str_release(vm_get_str(vm), topic_val.as.string);
    if (payload_val.type == VAL_STRING) str_release(vm_get_str(vm), payload_val.as.string);
    return err;
}

void stmt_mqtt_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MQTT",
        .category = "Wireless & IoT",
        .syntax = "MQTT.PUBLISH topic$, payload$ | MQTT.SUBSCRIBE topic$",
        .help_text = "Publishes telemetry or subscribes to message topics via MQTT broker.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
