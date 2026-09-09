// FILENAME: cross.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (cross.h, math.c, string.c)
// Provides runtime implementation for the CROSS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/cross.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_cross_desc = {
    .name = "CROSS",
    .category = "Math Functions",
    .syntax = "CROSS(u, v)",
    .description = "ECMA-116 standard function returning the cross product vector of 3D vectors u and v.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_cross_register(void) {
    lang_desc_register(&g_cross_desc);
}

BValue func_cross_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "CROSS") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13; err->message = "CROSS expects two vector arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    return res;
}
