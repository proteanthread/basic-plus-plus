// FILENAME: sleep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, sleep.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SLEEP statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/sleep.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include "platform/platform.h"
#include <string.h>

void stmt_sleep_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SLEEP",
        .category = "Control Flow",
        .syntax = "SLEEP [seconds]",
        .help_text = "Suspends program execution for the specified duration in seconds.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_sleep_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
        // Indefinite sleep or 1 second
        platform_sleep_ms(1000);
        return err;
    }

    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type != VAL_NUMBER && val.type != VAL_INTEGER) {
        if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    double sec = val.as.number;
    if (sec > 0.0) {
        uint32_t ms = (uint32_t)(sec * 1000.0);
        if (ms == 0) ms = 1;
        platform_sleep_ms(ms);
    }
    return err;
}
