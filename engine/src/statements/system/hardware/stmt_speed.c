// FILENAME: stmt_speed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, stmt_speed.h, func_baud.h, vm.h)
// Provides runtime implementation for the SPEED statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/stmt_speed.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_speed_desc = {
    .name = "SPEED",
    .category = "System & Hardware",
    .syntax = "SPEED [=] rate | SPEED% = val% | SPEED& = rate&",
    .description = "Configures console output throttling speed. Values <= 255 set Apple II delay scale (0..255); values > 255 or SPEED& set baud rate in bps (45.45 up to 115200+).",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_speed_register(void) {
    lang_desc_register(&g_speed_desc);
}

BppError stmt_speed_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool explicit_long_baud = false;
    bool explicit_apple_byte = false;

    // Check for optional type sigils '&' or '%' after SPEED
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_AMPERSAND) {
        explicit_long_baud = true;
        lex_next(lex);
        tok = lex_peek(lex);
    } else if (tok.type == TOK_MOD) {
        explicit_apple_byte = true;
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // Optional '=' assignment operator
    if (tok.type == TOK_EQ) {
        lex_next(lex);
    }

    // Evaluate speed expression
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (val.type != VAL_NUMBER && val.type != VAL_INTEGER) {
        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch for SPEED value";
        return err;
    }

    double speed_val = val.as.number;
    if (speed_val < 0.0) {
        err.code = 5;
        err.message = "Illegal function call: SPEED value cannot be negative";
        return err;
    }

    bool is_fractional_baud = (!explicit_apple_byte &&
        ((speed_val > 45.4 && speed_val < 45.5) ||
         (speed_val > 56.8 && speed_val < 56.9) ||
         (speed_val > 74.1 && speed_val < 74.3) ||
         (speed_val > 134.4 && speed_val < 134.6)));

    if (explicit_long_baud || is_fractional_baud || (!explicit_apple_byte && speed_val > 255.0)) {
        // Set console baud rate in bps (channel 0) and reset Apple delay
        baud_set_channel_rate(0, speed_val);
        speed_set_apple_speed(255.0);
    } else {
        // Set Apple II relative delay scale (0..255) and reset baud rate
        speed_set_apple_speed(speed_val);
        baud_set_channel_rate(0, 0.0);
    }

    return err;
}
