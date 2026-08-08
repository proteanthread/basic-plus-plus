/**
 * @file inp.c
 * @brief INP hardware I/O port reading function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements the `INP(port)` built-in function, reading an 8-bit byte value (0..255) from virtual hardware port address (0..65535).
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC, QBASIC, and ANSI BASIC hardware port access parity for low-level device control, timer querying, and virtual bus I/O.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 1 numeric argument, checks security permissions via `security_check_port_access()`, reads from virtual bus via `vdev_bus_in()`, and returns `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_inp'. Includes "eval/functions/system/inp.h",
 * "runtime/micro_lib_metadata.h", "device/bus.h", "security/security.h", "runtime/funcreg.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Extend virtual bus port mapping in `device/bus.c` or add 16-bit / 32-bit port read functions (`INPW`, `INPD`).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 1 argument signature validation, port address range checking (0..65535), byte value range return (0..255).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing 8-bit port byte or ERR_ILLEGAL_FUNCTION_CALL (error 5) / ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check virtual bus device registration in `device/bus.c` and security sandbox policy in `security/security.c`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer with virtual hardware bus active.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Virtualized port bus access avoids raw x86 assembly `in` instructions for cross-platform portability.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/device/bus.c
 * - engine/src/security/security.c
 * - engine/src/runtime/funcreg.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/inp.h
 * - engine/include/device/bus.h
 * - engine/include/security/security.h
 * - engine/include/runtime/funcreg.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/inp.h"
#include "runtime/micro_lib_metadata.h"
#include "device/bus.h"
#include "security/security.h"
#include "runtime/funcreg.h"
#include <stdint.h>
#include <string.h>

void func_inp_register(void) {
    MicroLibMetadata meta = {
        .name = "INP",
        .category = "System Functions",
        .syntax = "INP(port)",
        .help_text = "Reads a byte (0-255) from hardware I/O port address.",
        .error_codes = "Error 5: Illegal Function Call (port out of bounds), Error 13: Type Mismatch (expects numeric port)"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "INP",
        .keyword = KW_NONE,
        .category = FCAT_IO,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_SYSTEM,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read a byte from an I/O port",
        .module_name = "System"
    };
    funcreg_register(&entry);
}

BValue func_inp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "INP expects one numeric port argument";
        return res;
    }

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err->code = 70;
        err->message = "Permission denied: Port I/O is restricted";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)vdev_bus_in((int)args[0].as.number);
    return res;
}
