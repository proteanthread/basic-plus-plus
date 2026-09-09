// FILENAME: stmt_mqtt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements MQTT.PUBLISH and MQTT.SUBSCRIBE statements for IoT cloud messaging.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "iot_net.h"
#include "runtime/string/memops.h"

static const LangDesc g_mqtt_desc = {
    .name = "MQTT",
    .category = "Wireless & IoT",
    .syntax = "MQTT.PUBLISH topic$, payload$ | MQTT.SUBSCRIBE topic$",
    .description = "Publishes telemetry or subscribes to message topics via MQTT broker.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_mqtt_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_sub = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (tok_is_keyword(sub, KW_SUBSCRIBE, "SUBSCRIBE")) {
            is_sub = true;
            lex_next(lex);
        } else if (tok_is_keyword(sub, KW_PUBLISH, "PUBLISH")) {
            is_sub = false;
            lex_next(lex);
        } else if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
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
    lang_desc_register(&g_mqtt_desc);
}
