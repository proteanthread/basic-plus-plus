// FILENAME: spc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (spc.h, string.c, vm.h)
// Provides runtime implementation for the SPC built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/format/spc.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_spc_register(void) {
    MicroLibMetadata meta = {
        .name = "SPC",
        .category = "Print / Formatting Functions",
        .syntax = "SPC(n%)",
        .help_text = "Outputs or generates n space characters in a PRINT statement or expression.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPC expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_spc_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "SPC expects one numeric argument";
        return res;
    }

    int n = (int)args[0].as.number;
    if (n < 0) {
        err->code = 5;
        err->message = "Negative count in SPC";
        return res;
    }

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc((size_t)(n + 1)) : NULL);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    runtime_memset(buf, ' ', (size_t)n);
    buf[n] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm ? vm_get_str(vm) : NULL, buf, (size_t)n);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}
