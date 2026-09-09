// FILENAME: clamp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (clamp.h, math.c, string.c)
// Provides runtime implementation for the CLAMP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/clamp.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"

static const LangDesc g_clamp_desc = {
    .name = "CLAMP",
    .category = "Math Functions",
    .syntax = "CLAMP(val, min_val, max_val)",
    .description = "Clamps a value to be within the range [min_val, max_val].",
    .error_summary = "Error 13: Type Mismatch (CLAMP expects three numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_clamp_register(void) {
    lang_desc_register(&g_clamp_desc);
}

BValue func_clamp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_CLAMP") != 0 && runtime_strcmp(uname, "CLAMP") != 0 && runtime_strcmp(uname, "MATH.CLAMP") != 0) {
        return res;
    }

    if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "CLAMP expects three numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    double min_val = args[1].as.number;
    double max_val = args[2].as.number;

    if (x < min_val) x = min_val;
    if (x > max_val) x = max_val;

    res.type = VAL_NUMBER;
    res.as.number = x;
    return res;
}
