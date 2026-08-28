// FILENAME: and.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (and.h)
// Provides runtime implementation for the AND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/and.h"
#include "runtime/micro_lib_metadata.h"

void func_and_register(void) {
    MicroLibMetadata meta = {
        .name = "AND",
        .category = "Bitwise & Logical Functions",
        .syntax = "AND(val1, val2 [, ...]) or val1 AND val2",
        .help_text = "Performs bitwise and logical AND conjunction on integers or boolean values (supports dual prefix & infix notation).",
        .error_codes = "Error 13: Type Mismatch (AND expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_and_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_AND") != 0 && runtime_strcmp(uname, "AND") != 0 && runtime_strcmp(uname, "BITS.AND") != 0) {
        return res;
    }


    if (arg_count < 2) {
        err->code = 13;
        err->message = "AND expects at least two numeric arguments";
        return res;
    }

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "AND expects numeric arguments";
            return res;
        }
    }

    int64_t accum = (int64_t)args[0].as.number;
    for (int i = 1; i < arg_count; i++) {
        accum &= (int64_t)args[i].as.number;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)accum;
    return res;
}
