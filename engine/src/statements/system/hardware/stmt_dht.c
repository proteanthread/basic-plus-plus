// FILENAME: stmt_dht.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DHT.READ statement for digital temperature and humidity sensor reading.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "iot_sensors.h"
#include "runtime/variables.h"
#include <string.h>

BppError stmt_dht_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Optional .READ sub-token
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) ||
        ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 4 && runtime_strncasecmp(tok.start, "READ", 4) == 0)) {
        if (tok.type == TOK_PERIOD) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) ||
            ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length == 4 && runtime_strncasecmp(tok.start, "READ", 4) == 0)) {
            lex_next(lex);
        }
    }

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BppToken temp_tok = lex_next(lex);
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);
    BppToken hum_tok = lex_next(lex);

    int pin = (int)pin_val.as.number;
    double temp = 0.0;
    double hum = 0.0;
    iot_dht_read(pin, &temp, &hum);

    if (temp_tok.type == TOK_IDENT) {
        char tname[64];
        size_t tlen = (temp_tok.length < sizeof(tname) - 1) ? temp_tok.length : sizeof(tname) - 1;
        memcpy(tname, temp_tok.start, tlen);
        tname[tlen] = '\0';
        BValue tval;
        tval.type = VAL_NUMBER;
        tval.as.number = temp;
        var_assign(vm_get_var(vm), tname, tval);
    }

    if (hum_tok.type == TOK_IDENT) {
        char hname[64];
        size_t hlen = (hum_tok.length < sizeof(hname) - 1) ? hum_tok.length : sizeof(hname) - 1;
        memcpy(hname, hum_tok.start, hlen);
        hname[hlen] = '\0';
        BValue hval;
        hval.type = VAL_NUMBER;
        hval.as.number = hum;
        var_assign(vm_get_var(vm), hname, hval);
    }

    return err;
}

void stmt_dht_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DHT",
        .category = "Sensors & Actuators",
        .syntax = "DHT.READ pin, temp_var, hum_var",
        .help_text = "Reads temperature (Celsius) and relative humidity (%) from a DHT11 or DHT22 sensor.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
