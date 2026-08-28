// FILENAME: stmt_pinmode.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the PINMODE statement for microcontroller pin configuration.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_pinmode_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (pin_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), pin_val.as.string);
        err.code = 13;
        err.message = "Type mismatch: PINMODE expects numeric pin";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue mode_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int mode = 0;
    if (mode_val.type == VAL_STRING) {
        const char *mstr = str_data(mode_val.as.string);
        if (strcasecmp(mstr, "OUTPUT") == 0 || strcasecmp(mstr, "OUT") == 0) {
            mode = PIN_OUTPUT;
        } else if (strcasecmp(mstr, "INPUT_PULLUP") == 0 || strcasecmp(mstr, "PULLUP") == 0) {
            mode = PIN_INPUT_PULLUP;
        } else if (strcasecmp(mstr, "INPUT_PULLDOWN") == 0 || strcasecmp(mstr, "PULLDOWN") == 0) {
            mode = PIN_INPUT_PULLDOWN;
        } else {
            mode = PIN_INPUT;
        }
        str_release(vm_get_str(vm), mode_val.as.string);
    } else {
        mode = (int)mode_val.as.number;
    }

    int pin = (int)pin_val.as.number;
    esp32_hal_set_pin_mode(pin, mode);
    return err;
}

void stmt_pinmode_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PINMODE",
        .category = "Hardware & IoT",
        .syntax = "PINMODE pin, mode",
        .help_text = "Configures a microcontroller GPIO pin mode (INPUT, OUTPUT, PULLUP, PULLDOWN).",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
