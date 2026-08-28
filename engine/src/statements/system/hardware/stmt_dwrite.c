// FILENAME: stmt_dwrite.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DWRITE statement for digital pin output control.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_dwrite_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (pin_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), pin_val.as.string);
        err.code = 13;
        err.message = "Type mismatch: DWRITE expects numeric pin";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int output_val = 0;
    if (val.type == VAL_STRING) {
        const char *s = str_data(val.as.string);
        if (strcasecmp(s, "HIGH") == 0 || strcasecmp(s, "ON") == 0 || strcasecmp(s, "TRUE") == 0) {
            output_val = 1;
        } else {
            output_val = 0;
        }
        str_release(vm_get_str(vm), val.as.string);
    } else {
        output_val = (val.as.number != 0.0) ? 1 : 0;
    }

    int pin = (int)pin_val.as.number;
    esp32_hal_digital_write(pin, output_val);
    return err;
}

void stmt_dwrite_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DWRITE",
        .category = "Hardware & IoT",
        .syntax = "DWRITE pin, value",
        .help_text = "Writes a digital HIGH (1) or LOW (0) value to a microcontroller pin.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
