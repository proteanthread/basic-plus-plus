// FILENAME: radians.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, radians.h, string.c)
// Provides runtime implementation for the RADIANS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/radians.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_radians_desc = {
    .name = "RADIANS",
    .category = "Math Functions",
    .syntax = "RADIANS(x)",
    .description = "Converts angle x from degrees to radians (ANSI Full BASIC 1987).",
    .error_summary = "Error 13: Type Mismatch (RADIANS expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_radians_register(void) {
    lang_desc_register(&g_radians_desc);
}

BValue func_radians_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RADIANS") != 0 && runtime_strcmp(uname, "_RADIANS") != 0 &&
        runtime_strcmp(uname, "MATH.RADIANS") != 0 && runtime_strcmp(uname, "RAD") != 0 &&
        runtime_strcmp(uname, "RADIAN") != 0 &&
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
