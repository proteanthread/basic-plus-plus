// FILENAME: log10.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (log10.h, math.c, string.c)
// Provides runtime implementation for the LOG10 built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/log10.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_log10_desc = {
    .name = "LOG10",
    .category = "Math & Trigonometry",
    .syntax = "LOG10(x) | LGT(x)",
    .description = "Returns the base-10 logarithm of x (x > 0).",
    .error_summary = "Error 5: Illegal Function Call (x <= 0), Error 13: Type Mismatch (LOG10 expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_log10_register(void) {
    lang_desc_register(&g_log10_desc);
}

BValue func_log10_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LOG10") != 0 && runtime_strcmp(uname, "LGT") != 0 &&
        runtime_strcmp(uname, "_LOG10") != 0 && runtime_strcmp(uname, "MATH.LOG10") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "LOG10 expects one numeric argument";
        return res;
    }

    if (args[0].as.number <= 0.0) {
        err->code = 5;
        err->message = "Illegal function call: LOG10 argument must be positive";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_log10(args[0].as.number);
    return res;
}
