// FILENAME: radians.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, radians.h, string.c)
// Provides runtime implementation for the RADIANS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/radians.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_radians_register(void) {
    MicroLibMetadata meta = {
        .name = "RADIANS",
        .category = "Math Functions",
        .syntax = "RADIANS(x)",
        .help_text = "Converts angle x from degrees to radians (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch (RADIANS expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_radians_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RADIANS") != 0 && runtime_strcmp(uname, "_RADIANS") != 0 &&
        runtime_strcmp(uname, "MATH.RADIANS") != 0 && runtime_strcmp(uname, "RAD") != 0 &&
        runtime_strcmp(uname, "D2R") != 0 && runtime_strcmp(uname, "_D2R") != 0 &&
        runtime_strcmp(uname, "MATH.D2R") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "RADIANS expects one numeric argument";
        return res;
    }

    res.as.number = args[0].as.number * (M_PI / 180.0);
    res.type = VAL_NUMBER;

    return res;
}
