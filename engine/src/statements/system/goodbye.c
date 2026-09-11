// FILENAME: goodbye.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements the forced session exit statement GOODBYE in BASIC++.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "runtime/metadata.h"
#include "runtime/language_descriptor.h"
#include "runtime/task.h"
#include "types/errors.h"
#include "vm/vm.h"

static const LangDesc g_goodbye_desc = {
    .name = "GOODBYE",
    .category = "System & Environment",
    .syntax = "GOODBYE [exit_code]",
    .description = "Forcefully terminates all background tasks and aborts the session immediately.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

// registers microlib metadata for the GOODBYE statement
void stmt_goodbye_register(void) {
    lang_desc_register(&g_goodbye_desc);
}

// handles execution of the GOODBYE statement
BppError stmt_goodbye_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    // GOODBYE [code]: as BYE [code], but the forced-exit path -- background
    // tasks are killed rather than shut down. Bare GOODBYE is unchanged.
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_BACKSLASH) {
        BValue code_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (code_val.type != VAL_NUMBER && code_val.type != VAL_INTEGER) {
            if (code_val.type == VAL_STRING) str_release(vm_get_str(vm), code_val.as.string);
            err.code = 13;
            err.message = "Type mismatch: GOODBYE expects a numeric exit code";
            return err;
        }
        vm_set_exit_code(vm, (int)code_val.as.number);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        vdev_puts(vdev, "Forced exit.\n");
    }

    // Forcefully kill all active background tasks and jobs
    task_mgr_force_kill_all();

    // Abruptly close any open file channels
    FileContext *fc = vm_get_file(vm);
    if (fc) {
        file_close_all(fc);
    }

    // Halt and signal VM exit
    vm_halt(vm);
    vm_request_exit(vm);
    return err;
}
