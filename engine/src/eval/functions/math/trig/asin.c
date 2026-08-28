// FILENAME: asin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (asin.h, math.c, string.c)
// Provides runtime implementation for the ASIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/asin.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_asin_register(void) {
    MicroLibMetadata meta = {
        .name = "ASIN",
        .category = "Math Functions",
        .syntax = "ASIN(x)",
        .help_text = "Returns the arcsine of x in radians for -1.0 <= x <= 1.0.",
        .error_codes = "Error 5: Illegal Function Call (ASIN argument out of range [-1, 1]), Error 13: Type Mismatch (ASIN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_asin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_ASIN") != 0 && runtime_strcmp(uname, "ASIN") != 0 &&
        runtime_strcmp(uname, "MATH.ASIN") != 0 && runtime_strcmp(uname, "ASN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ASIN expects one numeric argument";
        return res;
    }

    if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
        err->code = 5;
        err->message = "ASIN domain error";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_asin(args[0].as.number);
    return res;
}
