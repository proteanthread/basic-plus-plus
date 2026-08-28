// FILENAME: stmt_i2c.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the I2C statement for Inter-Integrated Circuit bus read/write operations.
//
// ---- Includes ----

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_i2c_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int is_read = 0;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) ||
        ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) && tok.length >= 4)) {
        if (tok.type == TOK_PERIOD) {
            lex_next(lex);
            tok = lex_peek(lex);
        }
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_READ) ||
                (tok.length == 4 && runtime_strncasecmp(tok.start, "READ", 4) == 0)) {
                is_read = 1;
            }
            lex_next(lex);
        }
    }

    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue reg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    uint8_t addr = (uint8_t)addr_val.as.number;
    uint8_t reg = (uint8_t)reg_val.as.number;

    if (is_read) {
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);
        tok = lex_next(lex);
        if (tok.type == TOK_IDENT) {
            char var_name[64];
            size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, nlen);
            var_name[nlen] = '\0';
            int read_val = esp32_hal_i2c_read(addr, reg);
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = (double)read_val;
            var_assign(vm_get_var(vm), var_name, val);
        }
    } else {
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);
        BValue val_to_write = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        uint8_t val = (uint8_t)val_to_write.as.number;
        esp32_hal_i2c_write(addr, reg, val);
    }

    return err;
}

void stmt_i2c_register(void) {
    static const MicroLibMetadata meta = {
        .name = "I2C",
        .category = "Hardware & IoT",
        .syntax = "I2C.WRITE addr, reg, val | I2C.READ addr, reg, var",
        .help_text = "Performs read or write transaction over I2C hardware bus.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
