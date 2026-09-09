// FILENAME: grad.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, grad.h, string.c)
// Provides runtime implementation for the GRAD built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/grad.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_grad_fn_desc = {
    .name = "GRAD",
    .category = "Math Functions",
    .syntax = "GRAD(x)",
    .description = "Converts angle x from degrees to grads (100 grads = 90 degrees).",
    .error_summary = "Error 13: Type Mismatch (GRAD expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_grad_register(void) {
    lang_desc_register(&g_grad_fn_desc);
}

BValue func_grad_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "GRAD") != 0 && runtime_strcmp(uname, "GRADS") != 0 &&
        runtime_strcmp(uname, "_GRAD") != 0 && runtime_strcmp(uname, "MATH.GRAD") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "GRAD expects one numeric argument";
        return res;
    }

    res.as.number = args[0].as.number * (200.0 / 180.0);
    res.type = VAL_NUMBER;

    return res;
}
