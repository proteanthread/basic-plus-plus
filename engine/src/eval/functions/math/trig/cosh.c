// FILENAME: cosh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cosh.h, math.c, string.c)
// Provides runtime implementation for the COSH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/cosh.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_cosh_register(void) {
    static const MicroLibMetadata meta = {
        .name = "COSH",
        .category = "Math & Trigonometry",
        .syntax = "COSH(x) | HCS(x)",
        .help_text = "Returns the hyperbolic cosine of x.",
        .error_codes = "Error 13: Type Mismatch (COSH expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_cosh_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "COSH") != 0 && runtime_strcmp(uname, "HCS") != 0 &&
        runtime_strcmp(uname, "_COSH") != 0 && runtime_strcmp(uname, "MATH.COSH") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "COSH expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_cosh(args[0].as.number);
    return res;
}
