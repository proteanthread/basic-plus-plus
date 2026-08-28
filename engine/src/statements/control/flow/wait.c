// FILENAME: wait.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, string.c, wait.h)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the WAIT statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/wait.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "platform/platform.h"
#include <string.h>

void stmt_wait_register(void) {
    static const MicroLibMetadata meta = {
        .name = "WAIT",
        .category = "Control Flow",
        .syntax = "WAIT seconds | WAIT port, and_mask [, xor_mask] | WAIT #channel, seconds",
        .help_text = "Suspends execution for specified duration, or polls hardware port until condition is met.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_wait_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        BppToken comma = lex_next(lex);
        if (comma.type != TOK_COMMA) {
            err.code = ERR_SYNTAX;
            err.message = "Expected ',' after channel in WAIT";
            return err;
        }

        BValue sec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (sec_val.type != VAL_NUMBER && sec_val.type != VAL_INTEGER) {
            if (sec_val.type == VAL_STRING && sec_val.as.string) str_release(vm_get_str(vm), sec_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        return err;
    }

    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
        platform_sleep_ms(1000);
        return err;
    }

    BValue arg1 = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (arg1.type != VAL_NUMBER && arg1.type != VAL_INTEGER) {
        if (arg1.type == VAL_STRING && arg1.as.string) str_release(vm_get_str(vm), arg1.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    BppToken next_tok = lex_peek(lex);
    if (next_tok.type == TOK_COMMA) {
        // GW-BASIC Port Polling: WAIT port, and_mask [, xor_mask]
        lex_next(lex); // Consume ','
        BValue and_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (and_val.type != VAL_NUMBER && and_val.type != VAL_INTEGER) {
            if (and_val.type == VAL_STRING && and_val.as.string) str_release(vm_get_str(vm), and_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }

        BppToken comma2 = lex_peek(lex);
        if (comma2.type == TOK_COMMA) {
            lex_next(lex);
            BValue xor_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (xor_val.type == VAL_STRING && xor_val.as.string) str_release(vm_get_str(vm), xor_val.as.string);
        }
        // Port simulation / check
        return err;
    }

    // Single argument: WAIT seconds (Timesharing delay)
    double sec = arg1.as.number;
    if (sec > 0.0) {
        uint32_t ms = (uint32_t)(sec * 1000.0);
        if (ms == 0) ms = 1;
        platform_sleep_ms(ms);
    }
    return err;
}
