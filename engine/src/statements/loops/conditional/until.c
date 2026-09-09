// FILENAME: until.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, until.h)
// Provides runtime implementation for the UNTIL statement in BASIC++.
//
// ---- Includes ----

#include "statements/loops/conditional/until.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_until_desc = {
    .name = "UNTIL",
    .category = "Looping / Control Flow",
    .syntax = "UNTIL condition",
    .description = "Terminates a BASIC09 REPEAT...UNTIL loop block when condition becomes true.",
    .error_summary = "Error 2: Syntax Error, Error 32: UNTIL Without REPEAT",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_until_register(void) {
    lang_desc_register(&g_until_desc);
}

static bool until_val_is_truthy(BValue val) {
    if (val.type == VAL_STRING) {
        return (val.as.string && str_len(val.as.string) > 0);
    }
    return val.as.number != 0.0;
}

BppError stmt_until_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppLineNumber loop_line = 0;
    const char *loop_pos = NULL;
    if (!vm_do_peek(vm, &loop_line, &loop_pos)) {
        err.code = 32;
        err.message = "UNTIL without REPEAT/DO";
        return err;
    }

    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    bool truthy = until_val_is_truthy(cond);
    if (cond.type == VAL_STRING) {
        str_release(vm_get_str(vm), cond.as.string);
    }

    // REPEAT...UNTIL loops while FALSE, terminates when TRUE
    if (!truthy) {
        vm_jump(vm, loop_line, loop_pos);
    } else {
        vm_do_pop(vm, NULL, NULL);
    }

    return err;
}
