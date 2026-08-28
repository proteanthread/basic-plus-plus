// FILENAME: eps.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eps.h, math.c, string.c)
// Provides runtime implementation for the EPS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/eps.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_eps_register(void) {
    MicroLibMetadata meta = {
        .name = "EPS",
        .category = "Math Functions",
        .syntax = "EPS(x)",
        .help_text = "Returns the machine epsilon relative to x (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch (EPS expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_eps_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "EPS") != 0 && runtime_strcmp(uname, "_EPS") != 0 && runtime_strcmp(uname, "MATH.EPS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "EPS expects one numeric argument";
        return res;
    }

    double x = runtime_fabs(args[0].as.number);
    if (x == 0.0) {
        res.as.number = RUNTIME_DBL_EPSILON;
    } else {
        res.as.number = x * RUNTIME_DBL_EPSILON;
    }
    res.type = VAL_NUMBER;


    return res;
}
