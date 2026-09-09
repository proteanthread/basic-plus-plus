// FILENAME: tanh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, tanh.h)
// Provides runtime implementation for the TANH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/tanh.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_tanh_desc = {
    .name = "TANH",
    .category = "Math & Trigonometry",
    .syntax = "TANH(x) | HTN(x)",
    .description = "Returns the hyperbolic tangent of x.",
    .error_summary = "Error 13: Type Mismatch (TANH expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_tanh_register(void) {
    lang_desc_register(&g_tanh_desc);
}

BValue func_tanh_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "TANH") != 0 && runtime_strcmp(uname, "HTN") != 0 &&
        runtime_strcmp(uname, "_TANH") != 0 && runtime_strcmp(uname, "MATH.TANH") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "TANH expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_tanh(args[0].as.number);
    return res;
}
