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
#include <stdlib.h>
#include <string.h>

#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "runtime/metadata.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/task.h"
#include "types/errors.h"
#include "vm/vm.h"

// registers microlib metadata for the GOODBYE statement
void stmt_goodbye_register(void) {
    MicroLibMetadata meta = {
        .name = "GOODBYE",
        .category = "System & Environment",
        .syntax = "GOODBYE",
        .help_text = "Forcefully terminates all background tasks and aborts the session immediately.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

// handles execution of the GOODBYE statement
BppError stmt_goodbye_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_BACKSLASH) {
        err.code = 2;
        err.message = "Unexpected argument after GOODBYE";
        return err;
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

    // Immediate forced process exit
    exit(0);
}
