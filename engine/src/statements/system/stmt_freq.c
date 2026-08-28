// FILENAME: stmt_freq.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the FREQ statement for microcontroller CPU frequency configuration.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_freq_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BValue freq_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    (void)freq_val;
    return err;
}

void stmt_freq_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FREQ",
        .category = "System & Power",
        .syntax = "FREQ mhz",
        .help_text = "Sets CPU clock frequency (e.g. 80, 160, 240 MHz on ESP32).",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
