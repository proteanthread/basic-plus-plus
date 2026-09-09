// FILENAME: lerp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (lerp.h, math.c, string.c)
// Provides runtime implementation for the LERP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/lerp.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_lerp_desc = {
    .name = "LERP",
    .category = "Math Functions",
    .syntax = "LERP(a, b, t)",
    .description = "Performs linear interpolation between a and b using weight t (a + (b - a) * t).",
    .error_summary = "Error 13: Type Mismatch (LERP expects three numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_lerp_register(void) {
    lang_desc_register(&g_lerp_desc);
}

BValue func_lerp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_LERP") != 0 && runtime_strcmp(uname, "LERP") != 0 && runtime_strcmp(uname, "MATH.LERP") != 0) {
        return res;
    }

    if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "LERP expects three numeric arguments";
        return res;
    }

    double a = args[0].as.number;
    double b = args[1].as.number;
    double t = args[2].as.number;

    res.type = VAL_NUMBER;
    res.as.number = a + t * (b - a);
    return res;
}
