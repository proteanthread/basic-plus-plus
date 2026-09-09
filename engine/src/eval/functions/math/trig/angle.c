// FILENAME: angle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (angle.h, math.c, string.c)
// Provides runtime implementation for the ANGLE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/angle.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_angle_desc = {
    .name = "ANGLE",
    .category = "Math Functions",
    .syntax = "ANGLE(x, y)",
    .description = "Returns counterclockwise angle in radians from positive x-axis to (x, y) in [0, 2*pi) (ANSI Full BASIC 1987).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_angle_register(void) {
    lang_desc_register(&g_angle_desc);
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
