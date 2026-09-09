// FILENAME: stmt_pwm.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the PWM statement for Pulse Width Modulation frequency and duty control.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "esp32_hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_pwm_desc = {
    .name = "PWM",
    .category = "Hardware & IoT",
    .syntax = "PWM pin, freq, duty",
    .description = "Generates hardware Pulse Width Modulation with specified frequency (Hz) and duty cycle (0-1023).",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_pwm_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue freq_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue duty_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int pin = (int)pin_val.as.number;
    double freq = freq_val.as.number;
    double duty = duty_val.as.number;
    esp32_hal_pwm_write(pin, freq, duty);
    return err;
}

void stmt_pwm_register(void) {
    lang_desc_register(&g_pwm_desc);
}
