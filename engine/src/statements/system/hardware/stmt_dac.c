// FILENAME: stmt_dac.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DAC.WRITE statement for digital-to-analog converter voltage output.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_dac_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Optional .WRITE sub-token
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD || (tok.type == TOK_IDENT && tok.length == 5 && runtime_strncasecmp(tok.start, "WRITE", 5) == 0)) {
        if (tok.type == TOK_PERIOD) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_IDENT && tok.length == 5 && runtime_strncasecmp(tok.start, "WRITE", 5) == 0) {
            lex_next(lex);
        }
    }

    BValue pin_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (pin_val.type == VAL_STRING) {
        str_release(vm_get_str(vm), pin_val.as.string);
        err.code = 13;
        err.message = "Type mismatch: DAC expects numeric pin";
        return err;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int dac_val = (int)val.as.number;
    int pin = (int)pin_val.as.number;
    esp32_hal_dac_write(pin, dac_val);
    return err;
}

void stmt_dac_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DAC",
        .category = "Hardware & IoT",
        .syntax = "DAC pin, value | DAC.WRITE pin, value",
        .help_text = "Outputs an 8-bit analog voltage (0-255) on a hardware DAC pin (e.g. GPIO 25/26).",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
