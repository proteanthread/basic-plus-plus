// FILENAME: pause.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, pause.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the PAUSE statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/pause.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_pause_register(void) {
    MicroLibMetadata meta = {
        .name = "PAUSE",
        .category = "System & Execution",
        .syntax = "PAUSE [seconds | prompt$]",
        .help_text = "Pauses program execution or prompts user to press a key.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_pause_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            vdev_printf(vm_get_vdev(vm), "%s", str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            vdev_printf(vm_get_vdev(vm), "PAUSE: %g", val.as.number);
        }
    }
    return err;
}
