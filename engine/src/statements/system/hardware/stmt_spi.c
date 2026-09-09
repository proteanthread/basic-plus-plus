// FILENAME: stmt_spi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the SPI statement for Serial Peripheral Interface bus communication.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "esp32_hal.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_spi_desc = {
    .name = "SPI",
    .category = "Hardware & IoT",
    .syntax = "SPI.TRANSFER cs_pin, data$",
    .description = "Transfers data synchronously over the SPI serial bus with chip-select control.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_spi_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) lex_next(lex);
    }

    BValue cs_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue data_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int cs = (int)cs_val.as.number;
    if (data_val.type == VAL_STRING) {
        const char *s = str_data(data_val.as.string);
        size_t slen = str_len(data_val.as.string);
        uint8_t rx[256] = {0};
        esp32_hal_spi_transfer(cs, (const uint8_t *)s, rx, slen);
        str_release(vm_get_str(vm), data_val.as.string);
    }

    return err;
}

void stmt_spi_register(void) {
    lang_desc_register(&g_spi_desc);
}
