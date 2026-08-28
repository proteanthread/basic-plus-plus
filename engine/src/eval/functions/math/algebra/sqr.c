// FILENAME: sqr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, sqr.h, string.c)
// Provides runtime implementation for the SQR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/sqr.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_sqr_register(void) {
    MicroLibMetadata meta = {
        .name = "SQR",
        .category = "Math Functions",
        .syntax = "SQR(x)",
        .help_text = "Returns the non-negative square root of a numeric expression x >= 0.",
        .error_codes = "Error 5: Illegal Function Call (SQR of negative number), Error 13: Type Mismatch (SQR expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_sqr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "SQR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SQR expects one numeric argument";
        return res;
    }

    if (args[0].type == VAL_COMPLEX) {
        double r = runtime_hypot(args[0].as.complex_val.real, args[0].as.complex_val.imag);
        double theta = runtime_atan2(args[0].as.complex_val.imag, args[0].as.complex_val.real);
        double sqrt_r = runtime_sqrt(r);
        res.type = VAL_COMPLEX;
        res.as.complex_val.real = sqrt_r * runtime_cos(theta / 2.0);
        res.as.complex_val.imag = sqrt_r * runtime_sin(theta / 2.0);
        return res;
    }

    if (args[0].as.number < 0.0) {
        res.type = VAL_COMPLEX;
        res.as.complex_val.real = 0.0;
        res.as.complex_val.imag = runtime_sqrt(-args[0].as.number);
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_sqrt(args[0].as.number);
    return res;
}
