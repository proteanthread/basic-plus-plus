// FILENAME: space.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (space.h, string.c)
// Provides runtime implementation for the SPACE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/space.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_space_register(void) {
    MicroLibMetadata meta = {
        .name = "SPACE$",
        .category = "String Functions",
        .syntax = "SPACE$(n)",
        .help_text = "Returns a string consisting of n space characters.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPACE$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_space_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SPACE$ expects one numeric argument";
        return res;
    }

    int n = (int)args[0].as.number;
    if (n < 0) {
        err->code = 5;
        err->message = "Negative count in SPACE$";
        return res;
    }

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (n + 1)) : NULL);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    runtime_memset(buf, ' ', n);
    buf[n] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, n);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}
