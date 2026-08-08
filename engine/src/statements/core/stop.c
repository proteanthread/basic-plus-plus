/**
 * @file stop.c
 * @brief STOP statement handler for program pause and debug breakpoint handling.
 *
 * 1. WHAT IT DOES:
 * Implements STOP statement execution, rendering "Break in line X" to CON: and halting the VM.
 *
 * 2. WHY IT EXISTS:
 * Provides standard program execution pause capabilities and debugging breakpoints in BASIC++.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Retrieves current line number via vm_get_current_line(vm). Formats "Break in %lld\n" (casting line
 * double to long long to prevent scientific notation) to vdev, sets vm->state to VM_STATE_HALTED.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_stop'. Directly includes "statements/core/stop.h",
 * "vm/vm.h", "device/vdev.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To enable execution resumption via CONT, store current line and token offset in vm->cont_line.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Virtual console routing: break output MUST go through vdev_printf/vdev_puts. Line numbers display
 * as integers (%lld) not floating-point (%g).
 *
 * 8. WHAT TO EXPECT:
 * Modifies VM execution state to VM_STATE_HALTED and returns ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check vm_get_vdev() handle. Verify vm_halt() transition logic in vm/control.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext with active virtual device table.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Integer cast (long long) for 64-bit line numbers. Pure 7-bit ASCII.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/device/vdev.c
 * - engine/src/vm/control.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/stop.h
 * - engine/include/vm/vm.h
 * - engine/include/device/vdev.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/stop.h"
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
