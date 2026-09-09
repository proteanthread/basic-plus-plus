// FILENAME: runtime_tan.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, runtime_tan.h)
// Provides runtime implementation for the TAN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/tan.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "vm/vm.h"

static const LangDesc g_tan_desc = {
    .name = "TAN",
    .category = "Math Functions",
    .syntax = "TAN(angle)",
    .description = "Returns the trigonometric tangent of an angle (radians by default, degrees or grads if DEGREE/GRAD mode).",
    .error_summary = "Error 13: Type Mismatch (TAN expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_tan_register(void) {
    lang_desc_register(&g_tan_desc);
}

BValue func_tan_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "TAN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "TAN expects one numeric argument";
        return res;
    }

    double angle = args[0].as.number;
    int mode = vm_get_angle_mode(vm);
    if (mode == 1) {
        angle = angle * (3.14159265358979323846 / 180.0);
    } else if (mode == 2) {
        angle = angle * (3.14159265358979323846 / 200.0);
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_tan(angle);
    return res;
}
