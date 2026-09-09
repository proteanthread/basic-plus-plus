// FILENAME: stop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (stop.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the STOP statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/stop.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_stop_desc = {
    .name = "STOP",
    .category = "Control Flow",
    .syntax = "STOP",
    .description = "Pauses program execution and prints 'Break in line X' message.",
    .error_summary = "None (STOP halts program execution cleanly)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_stop_register(void) {
    lang_desc_register(&g_stop_desc);
}

BppError stmt_stop_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    VDevContext *vdev = vm_get_vdev(vm);
    BppLineNumber ln = vm_get_current_line(vm);
    if (ln > 0.0) {
        vdev_printf(vdev, "Break in %lld\n", (long long)ln);
    } else {
        vdev_puts(vdev, "Break\n");
    }
    vm_halt(vm);
    return err;
}
