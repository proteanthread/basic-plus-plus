// FILENAME: stmt_delay.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DELAY statement for millisecond and microsecond sleep pauses.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_delay_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_us = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT) {
            if (runtime_strncasecmp(sub.start, "US", sub.length) == 0) is_us = true;
            lex_next(lex);
        }
    }

    BValue d_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (is_us) {
        esp32_hal_delay_us((uint32_t)d_val.as.number);
    } else {
        esp32_hal_delay_ms((uint32_t)d_val.as.number);
    }
    return err;
}

void stmt_delay_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DELAY",
        .category = "Timing & Real-Time",
        .syntax = "DELAY ms | DELAY.MS ms | DELAY.US us",
        .help_text = "Pauses execution for specified milliseconds or microseconds.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
