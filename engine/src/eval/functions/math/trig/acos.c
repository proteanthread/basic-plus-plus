// FILENAME: runtime_acos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_acos.h, math.c, string.c)
// Provides runtime implementation for the ACOS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/acos.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "vm/vm.h"

static const LangDesc g_acos_desc = {
    .name = "ACOS",
    .category = "Math Functions",
    .syntax = "ACOS(x)",
    .description = "Returns the arccosine of x (in radians, degrees, or grads depending on angle mode) for -1.0 <= x <= 1.0.",
    .error_summary = "Error 5: Illegal Function Call (ACOS argument out of range [-1, 1]), Error 13: Type Mismatch (ACOS expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_acos_register(void) {
    lang_desc_register(&g_acos_desc);
}

BValue func_acos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_ACOS") != 0 && runtime_strcmp(uname, "ACOS") != 0 &&
        runtime_strcmp(uname, "MATH.ACOS") != 0 && runtime_strcmp(uname, "ACS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ACOS expects one numeric argument";
        return res;
    }

    if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
        err->code = 5;
        err->message = "ACOS domain error";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_acos(args[0].as.number);

    int mode = vm_get_angle_mode(vm);
    if (mode == 1) {
        res.as.number *= (180.0 / 3.14159265358979323846);
    } else if (mode == 2) {
        res.as.number *= (200.0 / 3.14159265358979323846);
    }

    return res;
}
