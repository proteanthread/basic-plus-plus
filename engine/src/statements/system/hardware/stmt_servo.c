// FILENAME: stmt_servo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the SERVO statement for hobby servo motor angular positioning.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_servo_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue angle_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int pin = (int)pin_val.as.number;
    double angle = angle_val.as.number;
    esp32_hal_servo_write(pin, angle);
    return err;
}

void stmt_servo_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SERVO",
        .category = "Hardware & IoT",
        .syntax = "SERVO pin, angle",
        .help_text = "Positions a servo motor on the specified pin to an angle between 0 and 180 degrees.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
