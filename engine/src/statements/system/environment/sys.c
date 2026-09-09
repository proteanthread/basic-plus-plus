// FILENAME: sys.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
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
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_sys_desc = {
    .name = "SYS",
    .category = "System & Execution",
    .syntax = "SYS address% | SYS command_string$",
    .description = "Executes machine code subroutine at memory address or runs host OS command.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_sys_register(void) {
    lang_desc_register(&g_sys_desc);
}

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

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
