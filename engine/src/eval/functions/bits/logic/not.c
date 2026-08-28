// FILENAME: not.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (not.h)
// Provides runtime implementation for the NOT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/not.h"
#include "runtime/micro_lib_metadata.h"

void func_not_register(void) {
    MicroLibMetadata meta = {
        .name = "NOT",
        .category = "Bitwise & Logical Functions",
        .syntax = "NOT(val) or NOT val",
        .help_text = "Performs bitwise and logical negation on an integer or boolean value.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_not_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_NOT") != 0 && runtime_strcmp(uname, "NOT") != 0 && runtime_strcmp(uname, "BITS.NOT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "NOT expects one numeric argument";
        return res;
    }

    int64_t a = (int64_t)args[0].as.number;

    res.type = VAL_NUMBER;
    res.as.number = (double)(~a);
    return res;
}
