// FILENAME: arg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (arg.h)
// Provides runtime implementation for the ARG function (JOSS / RAND P-2922).
//
// ---- Includes ----

#include "eval/functions/math/trig/arg.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/math/math.h"

static const LangDesc g_arg_desc = {
    .name = "ARG",
    .category = "Math Functions",
    .syntax = "ARG(x, y)",
    .description = "Returns the polar angle (argument) of coordinate (x, y) in radians (JOSS / RAND P-2922).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_arg_register(void) {
    lang_desc_register(&g_arg_desc);
}

BValue func_arg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 2 || (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER) ||
        (args[1].type != VAL_NUMBER && args[1].type != VAL_INTEGER)) {
        err->code = 13;
        err->message = "ARG expects two numeric arguments: ARG(x, y)";
        return res;
    }

    double x = args[0].as.number;
    double y = args[1].as.number;

    res.as.number = runtime_atan2(y, x);
    return res;
}
