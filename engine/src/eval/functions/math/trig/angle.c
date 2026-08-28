// FILENAME: angle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (angle.h, math.c, string.c)
// Provides runtime implementation for the ANGLE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/angle.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_angle_register(void) {
    MicroLibMetadata meta = {
        .name = "ANGLE",
        .category = "Math Functions",
        .syntax = "ANGLE(x, y)",
        .help_text = "Returns counterclockwise angle in radians from positive x-axis to (x, y) in [0, 2*pi) (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_angle_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "ANGLE") != 0 && runtime_strcmp(uname, "_ANGLE") != 0 && runtime_strcmp(uname, "MATH.ANGLE") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "ANGLE expects two numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    double y = args[1].as.number;

    if (x == 0.0 && y == 0.0) {
        res.as.number = 0.0;
    } else {
        double a = runtime_atan2(y, x);
        if (a < 0.0) {
            a += 2.0 * M_PI;
        }
        res.as.number = a;
    }
    res.type = VAL_NUMBER;

    return res;
}
