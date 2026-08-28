// FILENAME: shell.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, shell.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SHELL statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/environment/shell.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void stmt_shell_register(void) {
    MicroLibMetadata meta = {
        .name = "SHELL",
        .category = "System & OS",
        .syntax = "SHELL [command_string$]",
        .help_text = "Suspends the BASIC program and executes an operating system command shell.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BppError stmt_shell_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BValue cmd_val;
    memset(&cmd_val, 0, sizeof(cmd_val));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        platform_execute_shell();
        return err;
    }

    cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (cmd_val.type != VAL_STRING) {
        err.code = 13; err.message = "Type mismatch (expected string for SHELL)";
        return err;
    }

    if (cmd_val.as.string) {
        platform_execute_command(str_data(cmd_val.as.string));
        str_release(vm_get_str(vm), cmd_val.as.string);
    }
    return err;
}
