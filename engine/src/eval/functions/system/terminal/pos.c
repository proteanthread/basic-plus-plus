// FILENAME: pos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (pos.h, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the POS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/terminal/pos.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "device/vcon.h"

#include "runtime/string.h"
#include "runtime/memory.h"
void func_pos_register(void) {
    MicroLibMetadata meta = {
        .name = "POS",
        .category = "System / Screen Functions",
        .syntax = "POS(dummy%)",
        .help_text = "Returns the current horizontal column position of the cursor (1-indexed).",
        .error_codes = "Error 13: Type Mismatch (POS expects 1 argument)"
    };
    microlib_register(&meta);
}

BValue func_pos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    if (arg_count != 1) {
        err->code = 13;
        err->message = "POS expects 1 argument";
        return res;
    }

    int r = 0, c = 0;
    VConContext *vcon = vm ? vm_get_vcon(vm) : NULL;
    if (vcon) {
        vcon_get_cursor(vcon, 0, &r, &c);
        res.as.number = (double)(c + 1);
    }
    return res;
}
