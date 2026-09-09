// FILENAME: stmt_baud.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, stmt_baud.h, func_baud.h, vm.h)
// Provides runtime implementation for the BAUD statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/stmt_baud.h"
#include "eval/functions/system/hardware/func_baud.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_baud_desc = {
    .name = "BAUD",
    .category = "System & Hardware",
    .syntax = "BAUD rate# | BAUD = rate# | BAUD [#]channel%, rate#",
    .description = "Configures simulated transmission speed in bps (from vintage Baudot 45.45 bps up to 115200+ bps) on channel (0 for console).",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_baud_register(void) {
    lang_desc_register(&g_baud_desc);
}

BppError stmt_baud_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // Optional leading '#'
    BppToken tok = lex_peek(lex);
    bool has_hash = false;
    if (tok.type == TOK_HASH) {
        has_hash = true;
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // Optional '=' assignment operator
    if (tok.type == TOK_EQ) {
        lex_next(lex);
    }

    // Evaluate first expression (either channel or rate)
    BValue first_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (first_val.type != VAL_NUMBER && first_val.type != VAL_INTEGER) {
        if (first_val.type == VAL_STRING && first_val.as.string) {
            str_release(vm_get_str(vm), first_val.as.string);
        }
        err.code = 13;
        err.message = "Type mismatch for BAUD value";
        return err;
    }

    int channel = 0;
    double rate = first_val.as.number;

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // consume comma
        channel = (int)first_val.as.number;

        BValue rval = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (rval.type != VAL_NUMBER && rval.type != VAL_INTEGER) {
            if (rval.type == VAL_STRING && rval.as.string) {
                str_release(vm_get_str(vm), rval.as.string);
            }
            err.code = 13;
            err.message = "Type mismatch for BAUD rate";
            return err;
        }
        rate = rval.as.number;
    } else if (has_hash) {
        err.code = 2;
        err.message = "Expected ',' after BAUD channel number";
        return err;
    }

    if (rate < 0.0) {
        err.code = 5;
        err.message = "Illegal function call: BAUD rate cannot be negative";
        return err;
    }

    baud_set_channel_rate(channel, rate);
    if (channel == 0) {
        speed_set_apple_speed(255.0);
    }
    return err;
}
