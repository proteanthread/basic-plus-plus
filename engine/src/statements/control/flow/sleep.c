// FILENAME: sleep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, sleep.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SLEEP statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/flow/sleep.h"
#include "eval/eval.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_sleep_desc = {
    .name = "SLEEP",
    .category = "Control Flow",
    .syntax = "SLEEP [seconds]",
    .description = "Suspends program execution for the specified duration in seconds.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_sleep_register(void) {
    lang_desc_register(&g_sleep_desc);
}

static void interruptible_sleep(VMContext *vm, uint32_t ms, bool wait_for_key, BppError *err) {
    uint32_t elapsed = 0;
    while (true) {
        if (vm_break_triggered(vm) || platform_peek_key() == 3) {
            if (platform_peek_key() == 3) platform_inkey_char();
            vm_trigger_break(vm);
            err->code = ERR_DEVICE_IO_ERROR;
            err->message = "Break";
            return;
        }
        if (wait_for_key && platform_kbhit()) {
            platform_inkey_char();
            return;
        }
        if (!wait_for_key && elapsed >= ms) {
            return;
        }
        uint32_t step = (wait_for_key || (ms - elapsed > 10)) ? 10 : (ms - elapsed);
        platform_sleep_ms(step);
        elapsed += step;
    }
}

BppError stmt_sleep_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
        // Indefinite sleep until any key or Ctrl+C
        interruptible_sleep(vm, 0, true, &err);
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
        interruptible_sleep(vm, ms, false, &err);
    }
    return err;
}
