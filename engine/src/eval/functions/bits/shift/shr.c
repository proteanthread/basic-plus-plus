// FILENAME: shr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (shr.h)
// Provides runtime implementation for the SHR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/shift/shr.h"
#include "runtime/micro_lib_metadata.h"

void func_shr_register(void) {
    MicroLibMetadata meta = {
        .name = "SHR",
        .category = "Bitwise & Logical Functions",
        .syntax = "SHR(val, count) or val SHR count",
        .help_text = "Shifts an unsigned integer value right logically by the specified bit count.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_shr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_SHR") != 0 && runtime_strcmp(uname, "SHR") != 0 && runtime_strcmp(uname, "BITS.SHR") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "SHR expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number);
    return res;
}
