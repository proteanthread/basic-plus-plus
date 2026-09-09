// FILENAME: atn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (atn.h, math.c, string.c)
// Provides runtime implementation for the ATN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/atn.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"
#include "vm/vm.h"

static const LangDesc g_atn_desc = {
    .name = "ATN",
    .category = "Math Functions",
    .syntax = "ATN(x)",
    .description = "Returns the arctangent of a numeric expression (radians, degrees, or grads depending on angle mode).",
    .error_summary = "Error 13: Type Mismatch (ATN expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_atn_register(void) {
    lang_desc_register(&g_atn_desc);
}

BValue func_atn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "ATN") != 0 && runtime_strcmp(uname, "ATAN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ATN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_atan(args[0].as.number);

    int mode = vm_get_angle_mode(vm);
    if (mode == 1) {
        res.as.number *= (180.0 / 3.14159265358979323846);
    } else if (mode == 2) {
        res.as.number *= (200.0 / 3.14159265358979323846);
    }

    return res;
}
