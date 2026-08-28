// FILENAME: stmt_neopixel.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the NEOPIXEL statement for WS2812 addressable RGB LED control.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "iot_sensors.h"
#include <string.h>

BppError stmt_neopixel_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_show = false;
    bool is_clear = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "SHOW", 4) == 0) {
                is_show = true;
            }
            if ((sub.type == TOK_KEYWORD && (sub.as.keyword == KW_CLEAR || sub.as.keyword == KW_CLR)) ||
                (sub.length == 5 && runtime_strncasecmp(sub.start, "CLEAR", 5) == 0) ||
                (sub.length == 3 && runtime_strncasecmp(sub.start, "CLR", 3) == 0)) {
                is_clear = true;
            }
            lex_next(lex);
        }
    }

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int pin = (int)pin_val.as.number;

    if (is_show) {
        iot_neopixel_show(pin);
        return err;
    }
    if (is_clear) {
        iot_neopixel_clear(pin);
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);
    BValue idx_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);
    BValue r_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);
    BValue g_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);
    BValue b_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int index = (int)idx_val.as.number;
    uint8_t r = (uint8_t)r_val.as.number;
    uint8_t g = (uint8_t)g_val.as.number;
    uint8_t b = (uint8_t)b_val.as.number;

    iot_neopixel_set(pin, index, r, g, b);
    return err;
}

void stmt_neopixel_register(void) {
    static const MicroLibMetadata meta = {
        .name = "NEOPIXEL",
        .category = "Sensors & Actuators",
        .syntax = "NEOPIXEL pin, index, r, g, b | NEOPIXEL.SHOW pin | NEOPIXEL.CLEAR pin",
        .help_text = "Controls WS2812 / NeoPixel addressable RGB LED strip colors and latching.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
