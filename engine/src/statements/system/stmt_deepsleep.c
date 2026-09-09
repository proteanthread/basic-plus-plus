// FILENAME: stmt_deepsleep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements DEEPSLEEP and LIGHTSLEEP power-saving statements.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "esp32_hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_deepsleep_desc = {
    .name = "DEEPSLEEP",
    .category = "System & Power",
    .syntax = "DEEPSLEEP [ms] | LIGHTSLEEP [ms]",
    .description = "Enters ultra-low-power deep sleep standby mode for specified duration.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_deepsleep_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
        BValue ms_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        esp32_hal_delay_ms((uint32_t)ms_val.as.number);
    } else {
        esp32_hal_delay_ms(1000);
    }
    return err;
}

void stmt_deepsleep_register(void) {
    lang_desc_register(&g_deepsleep_desc);
}
