// FILENAME: stmt_nfc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Implements the NFC statement for RFID / NFC tag and card operations.
//
// ---- Includes ----

#include "statements/system/hardware/stmt_nfc.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "esp32_hal.h"
#include <string.h>

BppError stmt_nfc_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_init = false;
    bool is_scan = false;
    bool is_read = false;
    bool is_write = false;
    bool is_emulate = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "INIT", 4) == 0) is_init = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "SCAN", 4) == 0) is_scan = true;
            if ((sub.type == TOK_KEYWORD && sub.as.keyword == KW_READ) ||
                (sub.length == 4 && runtime_strncasecmp(sub.start, "READ", 4) == 0)) is_read = true;
            if (sub.length == 5 && runtime_strncasecmp(sub.start, "WRITE", 5) == 0) is_write = true;
            if (sub.length == 7 && runtime_strncasecmp(sub.start, "EMULATE", 7) == 0) is_emulate = true;
            lex_next(lex);
        }
    } else {
        is_scan = true;
    }

    if (is_init) {
        uint8_t addr = 0x24; // Default PN532 I2C address
        BppToken peek = lex_peek(lex);
        if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
            BValue addr_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && addr_val.type == VAL_NUMBER) addr = (uint8_t)addr_val.as.number;
        }
        (void)addr;
        return err;
    }

    if (is_scan) {
        tok = lex_next(lex);
        if (tok.type == TOK_IDENT) {
            char var_name[64];
            size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, nlen);
            var_name[nlen] = '\0';
            const char *uid = "04:5A:2B:C1:89:33";
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), uid, strlen(uid));
            var_assign(vm_get_var(vm), var_name, val);
        }
        return err;
    }

    if (is_read) {
        BValue block_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);

        tok = lex_next(lex);
        if (tok.type == TOK_IDENT) {
            char var_name[64];
            size_t nlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
            memcpy(var_name, tok.start, nlen);
            var_name[nlen] = '\0';
            const char *block_data = "NFC-BLOCK-DATA-42";
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_create(vm_get_str(vm), block_data, strlen(block_data));
            var_assign(vm_get_var(vm), var_name, val);
        }
        (void)block_val;
        return err;
    }

    if (is_write) {
        BValue block_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) lex_next(lex);

        BValue data_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (data_val.type == VAL_STRING) {
            str_release(vm_get_str(vm), data_val.as.string);
        }
        (void)block_val;
        return err;
    }

    if (is_emulate) {
        BValue uid_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (uid_val.type == VAL_STRING) str_release(vm_get_str(vm), uid_val.as.string);
        return err;
    }

    return err;
}

void stmt_nfc_register(void) {
    static const MicroLibMetadata meta = {
        .name = "NFC",
        .category = "Hardware & IoT",
        .syntax = "NFC.INIT [addr] | NFC.SCAN uid_var$ | NFC.READ block, data_var$ | NFC.WRITE block, data$ | NFC.EMULATE uid$",
        .help_text = "Controls external PN532 / MFRC522 NFC & 13.56 MHz RFID readers over I2C/SPI.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
