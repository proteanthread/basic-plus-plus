// FILENAME: cosh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (cosh.h, math.c, string.c)
// Provides runtime implementation for the COSH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/cosh.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_cosh_desc = {
    .name = "COSH",
    .category = "Math & Trigonometry",
    .syntax = "COSH(x) | HCS(x)",
    .description = "Returns the hyperbolic cosine of x.",
    .error_summary = "Error 13: Type Mismatch (COSH expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_cosh_register(void) {
    lang_desc_register(&g_cosh_desc);
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
