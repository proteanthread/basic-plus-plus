// FILENAME: runtime_ceil.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_ceil.h, math.c, string.c)
// Provides runtime implementation for the CEIL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/ceil.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

static const LangDesc g_ceil_desc = {
    .name = "CEIL",
    .category = "Math Functions",
    .syntax = "CEIL(x)",
    .description = "Returns the smallest integer greater than or equal to x.",
    .error_summary = "Error 13: Type Mismatch (CEIL expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_ceil_register(void) {
    lang_desc_register(&g_ceil_desc);
}

BValue func_ceil_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_CEIL") != 0 && runtime_strcmp(uname, "CEIL") != 0 && runtime_strcmp(uname, "MATH.CEIL") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "CEIL expects one numeric argument";
        return res;
    }

    double val = args[0].as.number;
    res.type = VAL_NUMBER;
    res.as.number = runtime_ceil(val);
    return res;
}
