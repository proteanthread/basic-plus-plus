// FILENAME: inp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (inp.h, string.c)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c)
// Provides runtime implementation for the INP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/hardware/inp.h"
#include "runtime/micro_lib_metadata.h"
#include "device/bus.h"
#include "security/security.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "runtime/memory.h"
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
