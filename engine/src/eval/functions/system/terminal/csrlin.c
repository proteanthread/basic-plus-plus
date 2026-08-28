// FILENAME: csrlin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (csrlin.h, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the CSRLIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/csrlin.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "device/vcon.h"

#include "runtime/string.h"
#include "runtime/memory.h"
void func_csrlin_register(void) {
    MicroLibMetadata meta = {
        .name = "CSRLIN",
        .category = "System / Screen Functions",
        .syntax = "row% = CSRLIN",
        .help_text = "Returns the current vertical line (row) position of the cursor (1-indexed).",
        .error_codes = "Error 13: Type Mismatch (CSRLIN expects no arguments)"
    };
    microlib_register(&meta);
}

BValue func_csrlin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CSRLIN expects no arguments";
        return res;
    }

    int r = 0, c = 0;
    VConContext *vcon = vm ? vm_get_vcon(vm) : NULL;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &r, &c);
        res.as.number = (double)(r + 1);
    }
    return res;
}
