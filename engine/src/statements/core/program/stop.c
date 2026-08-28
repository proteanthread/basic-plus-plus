// FILENAME: stop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (stop.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the STOP statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/stop.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include <string.h>

void stmt_stop_register(void) {
    MicroLibMetadata meta = {
        .name = "STOP",
        .category = "Control Flow",
        .syntax = "STOP",
        .help_text = "Pauses program execution and prints 'Break in line X' message.",
        .error_codes = "None (STOP halts program execution cleanly)"
    };
    microlib_register(&meta);
}

BppError stmt_stop_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));

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
