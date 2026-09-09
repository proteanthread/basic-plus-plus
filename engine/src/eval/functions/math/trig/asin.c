// FILENAME: runtime_asin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_asin.h, math.c, string.c)
// Provides runtime implementation for the ASIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/asin.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "vm/vm.h"

static const LangDesc g_asin_desc = {
    .name = "ASIN",
    .category = "Math Functions",
    .syntax = "ASIN(x)",
    .description = "Returns the arcsine of x (in radians, degrees, or grads depending on angle mode) for -1.0 <= x <= 1.0.",
    .error_summary = "Error 5: Illegal Function Call (ASIN argument out of range [-1, 1]), Error 13: Type Mismatch (ASIN expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_asin_register(void) {
    lang_desc_register(&g_asin_desc);
}

BValue func_asin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_ASIN") != 0 && runtime_strcmp(uname, "ASIN") != 0 &&
        runtime_strcmp(uname, "MATH.ASIN") != 0 && runtime_strcmp(uname, "ASN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ASIN expects one numeric argument";
        return res;
    }

    if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
        err->code = 5;
        err->message = "ASIN domain error";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_asin(args[0].as.number);

    int mode = vm_get_angle_mode(vm);
    if (mode == 1) {
        res.as.number *= (180.0 / 3.14159265358979323846);
    } else if (mode == 2) {
        res.as.number *= (200.0 / 3.14159265358979323846);
    }

    return res;
}
