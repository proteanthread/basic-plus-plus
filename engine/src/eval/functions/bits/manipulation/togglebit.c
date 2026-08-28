// FILENAME: togglebit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (togglebit.h)
// Provides runtime implementation for the TOGGLEBIT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/manipulation/togglebit.h"
#include "runtime/micro_lib_metadata.h"

void func_togglebit_register(void) {
    MicroLibMetadata meta = {
        .name = "TOGGLEBIT",
        .category = "Bitwise & Logical Functions",
        .syntax = "TOGGLEBIT(val, bit)",
        .help_text = "Inverts (flips) the bit at the specified zero-based bit index in an integer value.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_togglebit_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_TOGGLEBIT") != 0 && runtime_strcmp(uname, "TOGGLEBIT") != 0 && runtime_strcmp(uname, "BITS.TOGGLE") != 0) {
        return res;
    }


    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "TOGGLEBIT expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number ^ ((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    return res;
}
