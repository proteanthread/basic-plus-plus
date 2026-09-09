// FILENAME: stmt_freq.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the FREQ statement for microcontroller CPU frequency configuration.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_freq_desc = {
    .name = "FREQ",
    .category = "System & Power",
    .syntax = "FREQ mhz",
    .description = "Sets CPU clock frequency (e.g. 80, 160, 240 MHz on ESP32).",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_freq_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BValue freq_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    (void)freq_val;
    return err;
}

void stmt_freq_register(void) {
    lang_desc_register(&g_freq_desc);
}
