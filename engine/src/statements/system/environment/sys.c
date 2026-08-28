// FILENAME: sys.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, sys.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SYS statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/sys.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_sys_register(void) {
    MicroLibMetadata meta = {
        .name = "SYS",
        .category = "System & Execution",
        .syntax = "SYS address% | SYS command_string$",
        .help_text = "Executes machine code subroutine at memory address or runs host OS command.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF && tok.type != TOK_BACKSLASH) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_STRING && val.as.string) {
            platform_execute_command(str_data(val.as.string));
            str_release(vm_get_str(vm), val.as.string);
        }
    }
    return err;
}
