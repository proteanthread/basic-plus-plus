// FILENAME: pi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, pi.h, string.c)
// Provides runtime implementation for the PI built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/pi.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_pi_register(void) {
    MicroLibMetadata meta = {
        .name = "PI",
        .category = "Math Functions",
        .syntax = "PI()",
        .help_text = "Returns the mathematical constant pi (~3.141592653589793).",
        .error_codes = "Error 5: Illegal Function Call (PI expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_pi_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_PI") != 0 && runtime_strcmp(uname, "PI") != 0 && runtime_strcmp(uname, "MATH.PI") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 13;
        err->message = "PI expects no arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 3.14159265358979323846;
    return res;
}
