// FILENAME: cot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cot.h, math.c, string.c)
// Provides runtime implementation for the COT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/cot.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_cot_register(void) {
    static const MicroLibMetadata meta = {
        .name = "COT",
        .category = "Math & Trigonometry",
        .syntax = "COT(x)",
        .help_text = "Returns the cotangent of angle x in radians (1 / runtime_tan(x)).",
        .error_codes = "Error 11: Division by Zero (runtime_tan(x) == 0), Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_cot_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "COT") != 0 && runtime_strcmp(uname, "_COT") != 0 && runtime_strcmp(uname, "MATH.COT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "COT expects one numeric argument";
        return res;
    }

    double t = runtime_tan(args[0].as.number);
    if (t == 0.0) {
        err->code = 11;
        err->message = "Division by zero in COT";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 1.0 / t;
    return res;
}
