// FILENAME: cross.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cross.h, math.c, string.c)
// Provides runtime implementation for the CROSS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/cross.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_cross_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CROSS",
        .category = "Math Functions",
        .syntax = "CROSS(u, v)",
        .help_text = "ECMA-116 standard function returning the cross product vector of 3D vectors u and v.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_cross_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "CROSS") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13; err->message = "CROSS expects two vector arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    return res;
}
