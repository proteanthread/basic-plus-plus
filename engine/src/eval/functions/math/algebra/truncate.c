// FILENAME: truncate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, string.c, truncate.h)
// Provides runtime implementation for the TRUNCATE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/truncate.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_truncate_register(void) {
    MicroLibMetadata meta = {
        .name = "TRUNCATE",
        .category = "Math Functions",
        .syntax = "TRUNCATE(x [, n])",
        .help_text = "Truncates numeric x to n decimal places toward zero (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_truncate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "TRUNCATE") != 0 && runtime_strcmp(uname, "_TRUNCATE") != 0 &&
        runtime_strcmp(uname, "MATH.TRUNCATE") != 0 && runtime_strcmp(uname, "TRUNC") != 0 &&
        runtime_strcmp(uname, "_TRUNC") != 0 && runtime_strcmp(uname, "MATH.TRUNC") != 0) {
        return res;
    }

    if (arg_count < 1 || arg_count > 2 || args[0].type == VAL_STRING || (arg_count == 2 && args[1].type == VAL_STRING)) {
        err->code = 13;
        err->message = "TRUNCATE expects 1 or 2 numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    int n = 0;
    if (arg_count == 2) {
        n = (int)args[1].as.number;
    }

    if (n == 0) {
        res.as.number = runtime_trunc(x);
    } else {
        double p = runtime_pow(10.0, n);
        res.as.number = runtime_trunc(x * p) / p;
    }
    res.type = VAL_NUMBER;

    return res;
}
