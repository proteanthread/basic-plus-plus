// FILENAME: readbit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (readbit.h)
// Provides runtime implementation for the READBIT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/manipulation/readbit.h"
#include "runtime/micro_lib_metadata.h"

void func_readbit_register(void) {
    MicroLibMetadata meta = {
        .name = "READBIT",
        .category = "Bitwise & Logical Functions",
        .syntax = "READBIT(val, bit)",
        .help_text = "Returns the boolean bit value (0 or 1) at the specified zero-based bit index.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_readbit_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_READBIT") != 0 && runtime_strcmp(uname, "READBIT") != 0 && runtime_strcmp(uname, "BITS.READ") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "READBIT expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)(((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number) & 1);
    return res;
}
