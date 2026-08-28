// FILENAME: degrees.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (degrees.h, math.c, string.c)
// Provides runtime implementation for the DEGREES built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/degrees.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_degrees_register(void) {
    MicroLibMetadata meta = {
        .name = "DEGREES",
        .category = "Math Functions",
        .syntax = "DEGREES(x)",
        .help_text = "Converts angle x from radians to degrees (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch (DEGREES expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_degrees_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "DEGREES") != 0 && runtime_strcmp(uname, "_DEGREES") != 0 &&
        runtime_strcmp(uname, "MATH.DEGREES") != 0 && runtime_strcmp(uname, "DEG") != 0 &&
        runtime_strcmp(uname, "R2D") != 0 && runtime_strcmp(uname, "_R2D") != 0 &&
        runtime_strcmp(uname, "MATH.R2D") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "DEGREES expects one numeric argument";
        return res;
    }

    res.as.number = args[0].as.number * (180.0 / M_PI);
    res.type = VAL_NUMBER;

    return res;
}
